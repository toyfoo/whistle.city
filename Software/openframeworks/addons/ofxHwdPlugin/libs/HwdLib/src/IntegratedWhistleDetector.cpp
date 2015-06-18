/// \file
/// \brief IntegratedWhistleDetector class
/// \author Alexander Konin
/// \date 08.Apr.2015

#include "IntegratedWhistleDetector.h"
#include "Global.h"
#include "IntegratedWhistleDetectorGlobal.h"
#include "IntegratedWhistleDetector_p.h"
#include <algorithm>
#include <assert.h>
#include <math.h>

namespace Akon {
namespace HwdLib {

// IntegratedWhistleDetectorPrivate

IntegratedWhistleDetectorPrivate::IntegratedWhistleDetectorPrivate(IntegratedWhistleDetector& self,
	IntegratedWhistleDetector::WhistleEventHandler whistleEventHandler, void* userData) :
	self_(self),

	whistleEventHandler_(whistleEventHandler),
	userData_(userData),

	audio_(/*RtAudio::WINDOWS_DS*//*WINDOWS_WASAPI*/),  // WINDOWS_WASAPI gives input overflow
	workerThreadException_(),

	signalBuffer_(Akon::Fftw::checkedFftwMalloc<double>(SignalSize)),
	signalBufferCurrentSize_(0),

	fft_(FftOrder),
	fftOutput_(Akon::Fftw::checkedFftwMalloc<fftw_complex>(FftFrequencyCount)),
	powerSpectrum_(Akon::Fftw::checkedFftwMalloc<double>(FftFrequencyCount)),
	magnitudes_(Akon::Fftw::checkedFftwMalloc<double>(FftFrequencyCount)),

	framePowerComparator_(BinsPerUsingRange),
	windowSpikeDetector_(UsingRangeLowerBinIndex, BinsPerUsingRange, WhistleRangeLowerBinIndex,
		BinsPerWhistleRange),
	dyWaObertoneDetector_(SampleRate, SignalSize),
	whistleDetector_(BinsPerWhistleRange)
#ifdef USE_WEAK_SPIKES_SHADOWING
	, prevSpikeLevel_(0.0)
	, prevSpikeDurationCounter_(0)
#endif
#ifdef USE_2_CONSEQUITIVE_OBERTONES
	, prevObertoneBin_(Akon::HwdLib::VoidBin)
#endif
	, useOvertoneDetector_(true)
{
	if (!whistleEventHandler)
		throw std::invalid_argument("whistleEventHandler must be specified");

	if (audio_.getDeviceCount() < 1)
		throw std::runtime_error("No audio devices found");

	RtAudio::StreamParameters parameters;
	parameters.deviceId = audio_.getDefaultInputDevice();
	parameters.nChannels = 1;
	parameters.firstChannel = 0;
	unsigned int bufferFrames = SampleRate / 10;  // changed by RtAudio::openStream()

	//audio_.openStream(0, 0, RtAudioFormat(), 0, 0, 0, 0, 0, 0);
	audio_.openStream(0, &parameters, /*RTAUDIO_SINT16*/RTAUDIO_FLOAT64, SampleRate, &bufferFrames,
		&audioCallback, this);
}

IntegratedWhistleDetectorPrivate::~IntegratedWhistleDetectorPrivate()
{
}

int IntegratedWhistleDetectorPrivate::audioCallback(void* inputBuffer, unsigned int nBufferFrames,
	RtAudioStreamStatus status)
{
	//##note:
	// To continue normal stream operation, the RtAudioCallback function should return a value of
	// zero. To stop the stream and drain the output buffer, the function should return a value of
	// one. To abort the stream immediately, the client should return a value of two.

//	if (status)
//		std::cout << "status = " << status << std::endl;

	if (RTAUDIO_INPUT_OVERFLOW == status) {
		workerThreadException_.reset(new std::runtime_error("Stream underflow detected"));
		return 2;
	}

	processData(static_cast<double*>(inputBuffer), nBufferFrames);
	return 0;
}

void IntegratedWhistleDetectorPrivate::processData(const double* data, int size)
{
	for (int samplesProcessed = 0; samplesProcessed < size;) {
		// Skip samples up to next processing step
		if (signalBufferCurrentSize_ < 0) {
			const int samplesToSkip = std::min(size - samplesProcessed, - signalBufferCurrentSize_);
			signalBufferCurrentSize_ += samplesToSkip;
			samplesProcessed += samplesToSkip;
			continue;
		}

		// Accumulate data in 16-bytes aligned buffer
		const int samplesToCopy = std::min(size - samplesProcessed,
			SignalSize - signalBufferCurrentSize_);
		std::copy(data + samplesProcessed, data + samplesProcessed + samplesToCopy,
			&signalBuffer_[signalBufferCurrentSize_]);
		signalBufferCurrentSize_ += samplesToCopy;

		// Process accumulated data
		if (SignalSize == signalBufferCurrentSize_) {
			// Calculate FFT, power spectrum and amplitude spectrum.
			// Both IPP and FFTW have CCS output format:
			// R0 0 R1 I1 . . . RN/2-1 IN/2-1 RN/2 0 (total N/2+1 complex elements).
			// R1 I1 . . . RN/2-1 IN/2-1 have half value.
			assert(FftFrequencyCount == fft_.fftOutputSize());
			fft_(&signalBuffer_[0], &fftOutput_[0]);
			{
				struct Op
				{
					static double norm(fftw_complex elem)
					{
						return 2.0 * (elem[0] * elem[0] + elem[1] * elem[1]);
					}
				};
				std::transform(&fftOutput_[0], &fftOutput_[FftFrequencyCount],
					&powerSpectrum_[0], &Op::norm);
				powerSpectrum_[0] /= 2.0;
				powerSpectrum_[FftFrequencyCount - 1] /= 2.0;
			}
			{
				//##note: MSVC2010 cannot use ::sqrt() function in std::transform
				struct Op
				{
					static double sqrt(double elem) { return ::sqrt(elem); }

				};
				std::transform(&powerSpectrum_[0], &powerSpectrum_[FftFrequencyCount],
					&magnitudes_[0], &Op::sqrt);
			}

			// Invoke FramePowerComparator
			double framePower;
			const bool sufficientOnPowerFrame = framePowerComparator_(
				&powerSpectrum_[UsingRangeLowerBinIndex], framePower);

			// Invoke WindowSpikeDetector
			double spikeToSpikeAndSecondSpikeRatio;
			double secondSpikeBin;
			double spikeSharpness;
			double secondSpikeSharpness;
			double spikeLevel;  // 0.0 at VoidBin result
			double spikeBin = windowSpikeDetector_(&powerSpectrum_[UsingRangeLowerBinIndex],
				spikeToSpikeAndSecondSpikeRatio, secondSpikeBin,
				spikeSharpness, secondSpikeSharpness, spikeLevel);
				// spikeBin is relative to UsingRangeLowerBinIndex

#ifdef USE_WEAK_SPIKES_SHADOWING
			patchSpikeBinIfItsLevelIsTooSmall(spikeBin, spikeLevel);
#endif

			// Invoke DyWaObertoneDetector
			if (useOvertoneDetector_) {
				const double dyWaFundamentalFrequencyBin = dyWaObertoneDetector_(&signalBuffer_[0]);
				patchSpikeBinIfObertones(spikeBin, dyWaFundamentalFrequencyBin);
			}

			// Invoke WhistleDetector
			double whistleCertainty;
			int processingStepsPerWhistleDuration;
			const double whistleBin = whistleDetector_(sufficientOnPowerFrame ?
				spikeBin : Akon::HwdLib::VoidBin, spikeToSpikeAndSecondSpikeRatio,
				whistleCertainty, processingStepsPerWhistleDuration);
				// whistleBin is relative to UsingRangeLowerBinIndex

			if (Akon::HwdLib::VoidBin != whistleBin) {
				assert(whistleEventHandler_);  // mandatory callback
				(*whistleEventHandler_)(&self_,
					(samplesProcessed + samplesToCopy) / SamplesPerProcessingStep,
					(whistleBin + UsingRangeLowerBinIndex) * hzPerBin(),
					whistleCertainty,
					processingStepsPerWhistleDuration,
					userData_);
			}

			// Prepare next processing step
			signalBufferCurrentSize_ -= SamplesPerProcessingStep;
			if (signalBufferCurrentSize_ > 0) {
				const double* from = &signalBuffer_[SamplesPerProcessingStep];
				double* to = &signalBuffer_[0];
				std::copy(from, from + signalBufferCurrentSize_, to);
			}
		}

		samplesProcessed += samplesToCopy;
	}
}

void IntegratedWhistleDetectorPrivate::patchSpikeBinIfObertones(double& spikeBin, double obertoneBin)
{
#ifdef USE_2_CONSEQUITIVE_OBERTONES
	// Patch spikeBin if obertones are detected (patch condition - >=2 consequitive obertones)

	static const double ObertonesDeviationThreshold = 0.15;//0.1;

	if (Akon::HwdLib::VoidBin != spikeBin && Akon::HwdLib::VoidBin != obertoneBin &&
		Akon::HwdLib::VoidBin != prevObertoneBin_ &&
		fabs(obertoneBin - prevObertoneBin_) / prevObertoneBin_ <= ObertonesDeviationThreshold) {
		//#note: Akon's whistle has obertones (with DyWa) about 0.71
		static const double F0Threshold = 0.65;
		if (obertoneBin <= F0Threshold * (spikeBin + UsingRangeLowerBinIndex))
			spikeBin = Akon::HwdLib::VoidBin;
	}
	prevObertoneBin_ = obertoneBin;
#else
	// Patch spikeBin if obertones are detected
	if (Akon::HwdLib::VoidBin != spikeBin && Akon::HwdLib::VoidBin != obertoneBin) {
		//#note: Akon's whistle has obertones (with DyWa) about 0.71
		static const double F0Threshold = 0.65;
		if (obertoneBin <= F0Threshold * (spikeBin + UsingRangeLowerBinIndex))
			spikeBin = Akon::HwdLib::VoidBin;
	}
#endif
}

#ifdef USE_WEAK_SPIKES_SHADOWING
/// Patches \a spikeBin if its \a spikeLevel (power or amplitude) is too small relatively the
/// previous one.
void IntegratedWhistleDetectorPrivate::patchSpikeBinIfItsLevelIsTooSmall(double& spikeBin,
	double spikeLevel)
{
	if (0 == prevSpikeDurationCounter_ || prevSpikeLevel_ <= spikeLevel) {
		static const int ShadowDuration = 8;//4;
		prevSpikeLevel_ = spikeLevel;
		prevSpikeDurationCounter_ = ShadowDuration - 1;
	} else {
		static const double SpikeLevelThreshold = 0.05;//0.1;
		if (spikeLevel < prevSpikeLevel_ * SpikeLevelThreshold)
			spikeBin = Akon::HwdLib::VoidBin;
		--prevSpikeDurationCounter_;
	}
}
#endif

// IntegratedWhistleDetector

/*!
	\class IntegratedWhistleDetectorPrivate
	\brief Detects separate whistles in a signal, includes audio input and FFT
*/

/// Constructs the class instance.
/// \param whistleEventHandler - a user callback to be called at each whistle detection;
/// \param usedData - an arbitrary user data passed back in the callback (typically, this is
///   pointer to some user object).
/// throws RtAudioError
/// throws std::invalid_argument
/// throws std::runtime_error
IntegratedWhistleDetector::IntegratedWhistleDetector(WhistleEventHandler whistleEventHandler,
	void* userData) :
	pimpl_(new IntegratedWhistleDetectorPrivate(*this, whistleEventHandler, userData))
{
}

IntegratedWhistleDetector::~IntegratedWhistleDetector()
{
	close();
	delete pimpl_;
}

double IntegratedWhistleDetector::msecsPerProcessingStep()
{
	return Akon::HwdLib::msecsPerProcessingStep();
}

int IntegratedWhistleDetector::hzPerWindow() const
{
	return pimpl_->windowSpikeDetector_.binsPerWindow() * hzPerBin();
}

void IntegratedWhistleDetector::setHzPerWindow(int value)
{
	pimpl_->windowSpikeDetector_.setBinsPerWindow(value / hzPerBin());
}

int IntegratedWhistleDetector::whistlePowerThresholdPercent() const
{
	return pimpl_->whistleDetector_.powerThresholdPercent();
}

void IntegratedWhistleDetector::setWhistlePowerThresholdPercent(int value)
{
	pimpl_->whistleDetector_.setPowerThresholdPercent(value);
}

int IntegratedWhistleDetector::msecsPerWhistleDuration() const
{
	return pimpl_->whistleDetector_.stepsPerWhistleDuration() * msecsPerProcessingStep();
}

void IntegratedWhistleDetector::setMsecsPerWhistleDuration(int value)
{
	pimpl_->whistleDetector_.setStepsPerWhistleDuration(value / msecsPerProcessingStep());
}

int IntegratedWhistleDetector::hzPerWhistleFrequencyDeviation() const
{
	return pimpl_->whistleDetector_.binsPerFrequencyDeviation() * hzPerBin();
}

void IntegratedWhistleDetector::setHzPerWhistleFrequencyDeviation(int value)
{
	return pimpl_->whistleDetector_.setBinsPerFrequencyDeviation(value / hzPerBin());
}

float IntegratedWhistleDetector::framePowerThreshold() const
{
	return pimpl_->framePowerComparator_.threshold();
}

void IntegratedWhistleDetector::setFramePowerThreshold(float value)
{
	return pimpl_->framePowerComparator_.setThreshold(value);
}
	
bool IntegratedWhistleDetector::useOvertoneDetector() const
{
	return pimpl_->useOvertoneDetector_;
}
	
void IntegratedWhistleDetector::setUseOvertoneDetector(bool value)
{
	if (pimpl_->useOvertoneDetector_ == value) return;
	pimpl_->useOvertoneDetector_ = value;
}

/// Returns flag indicating opened/closed state.
bool IntegratedWhistleDetector::isOpened() const
{
	return pimpl_->audio_.isStreamRunning();
}
	
/// Opens underlying audio input and starts processing input signal.
/// throws RtAudioError if something wrong with underlying audio input (RtAudio)
void IntegratedWhistleDetector::open()
{
	if (isOpened()) return;
	pimpl_->workerThreadException_.reset();
	pimpl_->audio_.startStream();
}

/// Stops processing and closes underlying audio input.
void IntegratedWhistleDetector::close()
{
	if (!isOpened()) return;
	pimpl_->audio_.stopStream();
}

/// This exception is setted if the internal worker thread has finished with an error;
/// otherwise this is zero pointer. Check the exception after the thread has finished (at this
/// /*openChanged() signal occurs and */isOpen() returns false). The exception remains valid until
/// open() is called.
/// \note This function exposes only internal worker thread exceptions. Exceptions from other
/// places (for instance, from open()) are not covered by this function.
const std::runtime_error* IntegratedWhistleDetector::workerThreadException() const
{
	return pimpl_->workerThreadException_.get();
}

}  // HwdLib::
}  // Akon::


/* ==TRASH==
*/
