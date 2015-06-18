/// \file
/// \brief WindowSpikeDetector class
/// \author Alexander Konin
/// \date 08.Oct.2014

#include "WindowSpikeDetector.h"
#include "Global.h"
#include "Utils.h"
#ifdef USE_IPP
	#include <Akon/Ipp/Utils.h>
#else
	#include <numeric>  // std::accumulate
#endif
#include <algorithm>  // std::max_element
#include <math.h>
#include <assert.h>

namespace Akon {
namespace HwdLib {

// Utils

/// Returns max sum window with \a windowSize size that covers [\a lowerIndex, \a upperIndex]
/// subrange (i.e. these points lie within the window). Through \a windowIndex (if assigned)
/// returned window first element index. If \a windowSize exceeds \a dataSize, it is truncated.
/// If !(upperIndex >= lowerIndex) or !(windowSize >= upperIndex - lowerIndex) the function leads
/// to undefined behaviour. If [\a lowerIndex, \a upperIndex] subrange does not intersect with
/// data (i.e. both indexes are negative or >= \a dataSize), the function returns 0.0.
static double findMaxSumWindowCoveringIndexes(const double* data, int dataSize,
	int lowerIndex, int upperIndex, int windowSize, int* windowIndex = 0)
{
	windowSize = std::min(windowSize, dataSize);

	assert(upperIndex >= lowerIndex);
	assert(windowSize >= upperIndex - lowerIndex);

	const auto begin = data + std::max(upperIndex - (windowSize - 1), 0);
	const auto end = data + std::min(lowerIndex + 1, dataSize - windowSize + 1);

	//##note: If upperIndex < 0 or lowerIndex >= dataSize then begin >= end.
	if (begin >= end) {
		if (windowIndex)
			*windowIndex = dataSize;
		return 0.0;
	}

	auto window = begin;
	double energy = sum(window, windowSize);
	auto maxWindow = window;
	double maxEnergy = energy;
	++window;

	for (; window < end; ++window) {
		const auto removingDatum = window[-1];
		const auto addingDatum = window[-1 + windowSize];
		energy = energy - removingDatum + addingDatum;

		if (maxEnergy < energy) {
			maxWindow = window;
			maxEnergy = energy;
		}
	}

	if (windowIndex)
		*windowIndex = maxWindow - data;
	return maxEnergy;
}

static void findMaxSumWindowsCoveringIndex(const double* data, int dataSize, int index,
	int windowSize, int extendedWindowSize, double& windowSum, double& extendedWindowSum)
{
	assert(windowSize <= extendedWindowSize);

	int windowIndex;
	windowSum = findMaxSumWindowCoveringIndexes(data, dataSize, index, index, windowSize,
		&windowIndex);
	extendedWindowSum = findMaxSumWindowCoveringIndexes(data, dataSize, windowIndex,
		windowIndex + windowSize - 1, extendedWindowSize);
}

/// Returns max element's index in range: [0, \a exceptDataOffset) and
/// [exceptDataOffset + exceptDataSize, dataSize). Returns \a dataSize if there is no place where
/// to find the max (i.e. there are no elements remain after data excepting). If there is no max
/// (all values are equal), returns first allowed element's index.
static int findMaxInTwoRanges(const double* data, int dataSize,
	int exceptDataOffset, int exceptDataSize)
{
	if (exceptDataOffset < 0) {
		exceptDataSize += exceptDataOffset;
		exceptDataOffset = 0;
	}
	if (exceptDataOffset + exceptDataSize > dataSize)
		exceptDataSize = dataSize - exceptDataOffset;

	const auto leftMax = std::max_element(&data[0], &data[exceptDataOffset]);
	const auto rightMax = std::max_element(&data[exceptDataOffset + exceptDataSize], &data[dataSize]);

	const bool leftMaxValid = &data[exceptDataOffset] != leftMax;
	const bool rightMaxValid = &data[dataSize] != rightMax;

	auto result = &data[dataSize];  // invalid
	if (leftMaxValid && rightMaxValid)
		result = *leftMax >= *rightMax ? leftMax : rightMax;
	else if (leftMaxValid && !rightMaxValid)
		result = leftMax;
	else if (!leftMaxValid && rightMaxValid)
		result = rightMax;
	return result - data;
}

/// Returns offset from \a index that corresponds to max element: -1, if left > right;
/// 0, if left == right; -1 if left < right. If there is only left or right element, returns -1 or 1
/// respectively. Leads to undefined behaviour if \dataSize <= 1 or \a index is out of range.
static int getMaxElementOffset(const double* data, int dataSize, int index)
{
	assert(dataSize > 1);
	assert(index >= 0 && index < dataSize);

	const bool haveLeftElement = index > 0;
	const bool haveRightElement = index < dataSize - 1;

	//#note: either haveLeftElement or haveRightElement or both are true if dataSize > 1 (asserted)

	int result = -1;  // assume left
	if (haveLeftElement && haveRightElement) {
		if (data[index - 1] < data[index + 1])
			result = 1;
		else if (data[index - 1] == data[index + 1])
			result = 0;
	} else if (haveRightElement)
		result = 1;
	return result;
}

/// Assumes \a data is power spectrum (sqrt() is used).
static double getRefinedIndex(const double* data, int dataSize, int index)
{
	const int offset = getMaxElementOffset(data, dataSize, index);  // -1, 0 or 1
	if (0 == offset)
		return index;

	auto valueUnderIndex = data[index];
	auto valueUnderOffset = data[index + offset];

	// Assume 0.0 as baseline, i.e. assume data elements >= 0.0
	assert(valueUnderIndex >= 0.0);
	assert(valueUnderOffset >= 0.0);

	valueUnderIndex = sqrt(valueUnderIndex);
	valueUnderOffset = sqrt(valueUnderOffset);

	const auto sum = valueUnderIndex + valueUnderOffset;

	// Assume at least one element is non-zero
	assert(sum > 0.0);

	return index + offset * valueUnderOffset / sum;
}

/*!
	\class WindowSpikeDetector
	\brief Performs STFT frame analysis
*/

// WindowSpikeDetector

WindowSpikeDetector::WindowSpikeDetector(int binsPerUsingRangeOffset, int binsPerUsingRange,
	int binsPerWhistleRangeOffset, int binsPerWhistleRange) :
	binsPerUsingRangeOffset_(binsPerUsingRangeOffset),
	binsPerUsingRange_(binsPerUsingRange),
	binsPerWhistleFromUsingRangeOffset_(binsPerWhistleRangeOffset - binsPerUsingRangeOffset),
	binsPerWhistleRange_(binsPerWhistleRange),
	binsPerWindow_(std::min(3, binsPerWhistleRange)),
	contrastedSpectrum_(new double[binsPerUsingRange])
{
}

void WindowSpikeDetector::setBinsPerWindow(int value)
{
	//value = qBound(1, value, binsPerWhistleRange_);
	if (value < 1)
		value = 1;
	if (value > binsPerWhistleRange_)
		value = binsPerWhistleRange_;

	if (binsPerWindow_ == value) return;
	binsPerWindow_ = value;
	//Q_EMIT binsPerWindowChanged(binsPerWindow_);
}

/// ?Process one STFT block. Returns max frequency bin index of window with max energy or VoidBin if
/// input is zero. The index is counted from zero, i.e. min valid index is lookingRangeFirstBin().
/// \param powerSpectrum - ...
double WindowSpikeDetector::operator()(const double* usingRangePowerSpectrum,
	double& spikeToSpikeAndSecondSpikeRatio, double& secondSpike,
	double& spikeSharpness, double& secondSpikeSharpness, double& spikeLevel)
{
	// Params may change from other (user) thread, check that they change with atomic instruction
	// (mov/add m32/m64), i.e. they have size <= sizeof(void*)
	static_assert(sizeof(binsPerWindow_) <= sizeof(void*), "");

	// Use latched values due to original values may change from other (user) thread
	const int latchedBinsPerWindow = binsPerWindow_;

	spikeToSpikeAndSecondSpikeRatio = VoidRatio;
	secondSpike = VoidBin;
	spikeSharpness = VoidRatio;
	secondSpikeSharpness = VoidRatio;
	spikeLevel = 0.0;

	const int extendedWindowSize = latchedBinsPerWindow * 2;
	const int spikeGap = latchedBinsPerWindow;

	// Find spike index within whistle range
	const int spikeIndex = std::max_element(
		&usingRangePowerSpectrum[binsPerWhistleFromUsingRangeOffset_],
		&usingRangePowerSpectrum[binsPerWhistleFromUsingRangeOffset_] + binsPerWhistleRange_) -
		&usingRangePowerSpectrum[0];

	// Check we have valid spike, i.e. there are no a constant value across whistle range - at this
	// there is no maximum and std::max_element returns first element
	assert(binsPerWhistleRange_ > 1);  // for powerSpectrum[spikeIndex + 1]
	const bool spikeValid = spikeIndex != binsPerWhistleFromUsingRangeOffset_ ||
		usingRangePowerSpectrum[spikeIndex] != usingRangePowerSpectrum[spikeIndex + 1];
	if (!spikeValid) return VoidBin;

	// Find whistle range spike window and extended window energies (extended window is used for
	// finding spike sharpness)
	double spikeWindowEnergy;
	double spikeExtendedWindowEnergy;
	findMaxSumWindowsCoveringIndex(&usingRangePowerSpectrum[0], binsPerUsingRange_, spikeIndex,
		latchedBinsPerWindow, extendedWindowSize, spikeWindowEnergy, spikeExtendedWindowEnergy);

	// Find second spike index elsewhere, except the gap around the first spike index
	const int secondSpikeIndex = findMaxInTwoRanges(&usingRangePowerSpectrum[0], binsPerUsingRange_,
		spikeIndex - spikeGap, latchedBinsPerWindow + 2 * spikeGap);
		// Returns binsPerRange_ (dataSize) if there is no place to find the max
	const bool secondSpikeValid = binsPerUsingRange_ != secondSpikeIndex;

	// Find second spike windows energies
	double secondSpikeWindowEnergy;
	double secondSpikeExtendedWindowEnergy;
	findMaxSumWindowsCoveringIndex(&usingRangePowerSpectrum[0], binsPerUsingRange_, secondSpikeIndex,
		latchedBinsPerWindow, extendedWindowSize, secondSpikeWindowEnergy,
		secondSpikeExtendedWindowEnergy);
		// Returns 0.0s if secondSpikeIndex is invalid

	const double refinedSpikeIndex = getRefinedIndex(&usingRangePowerSpectrum[0],
		binsPerUsingRange_, spikeIndex);
	if (secondSpikeValid)
		secondSpike = getRefinedIndex(&usingRangePowerSpectrum[0],
			binsPerUsingRange_, secondSpikeIndex);

	// Here we have non-constant value across whistle range, and spikeWindowEnergy cannot be zero,
	// because we have valid spike
	assert(spikeWindowEnergy > 0.0);
	assert(spikeExtendedWindowEnergy > 0.0);

	spikeToSpikeAndSecondSpikeRatio = spikeWindowEnergy / (spikeWindowEnergy + secondSpikeWindowEnergy);
	spikeSharpness = spikeWindowEnergy / spikeExtendedWindowEnergy;
	if (secondSpikeValid)
		secondSpikeSharpness = secondSpikeWindowEnergy / secondSpikeExtendedWindowEnergy;

	spikeLevel = spikeWindowEnergy;
	return refinedSpikeIndex;
}

}  // HwdLib::
}  // Akon::


/* ==TRASH==
*/
