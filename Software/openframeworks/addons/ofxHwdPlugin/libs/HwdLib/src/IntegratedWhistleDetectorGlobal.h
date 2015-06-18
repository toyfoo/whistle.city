/// \file
/// \brief Global info
/// \author Alexander Konin
/// \date 22.Sep.2014

#pragma once

namespace Akon {
namespace HwdLib {

/// Sampling rate
static const int SampleRate = 44100;  // Hz

/// FFT block order, 44100 / 4096 = 10.7666 -> ~93 msec., 44100 / 1024 = 43.0664 -> ~23 msec.
/// HwdPlugin v0.5.1: FftOrder = 10
static const int FftOrder = 10;

/// Signal size in samples for FFT processing
static const int SignalSize = 1 << FftOrder;

/// Count of frequencies [0..N/2] in using FFT block
static const int FftFrequencyCount = SignalSize / 2 + 1;  // freqs. [0..N/2]

/// Returns FFT resolution
inline double hzPerBin() { return double(SampleRate) / SignalSize; }

/// \name Using range (range with discarded low and high frequencies as BP filtering) references
//@{
static const int UsingRangeLowerFrequencyRef = 800;//400;//800; // 600// Hz
static const int UsingRangeUpperFrequencyRef = 8000;//3000;//8000;  // Hz
static_assert(UsingRangeUpperFrequencyRef <= SampleRate / 2, "");
//@}

/// \name Using range values
//@{
static const int UsingRangeLowerBinIndex = UsingRangeLowerFrequencyRef * SignalSize / SampleRate;
static const int UsingRangeUpperBinIndex = UsingRangeUpperFrequencyRef * SignalSize / SampleRate;
static const int BinsPerUsingRange = UsingRangeUpperBinIndex - UsingRangeLowerBinIndex + 1;
static_assert(BinsPerUsingRange, "");
inline double usingRangeLowerFrequency() { return hzPerBin() * UsingRangeLowerBinIndex; }
inline double usingRangeUpperFrequency() { return hzPerBin() * UsingRangeUpperBinIndex; }
inline double hzPerUsingRange() { return hzPerBin() * BinsPerUsingRange; }
//@}

/// \name Whistle range (where a whiste is being searched, 500 - 5000 Hz generally) references
/// HwdPlugin v0.5.1: [1250; 4000]
//@{
static const int WhistleRangeLowerFrequencyRef = 1250;  // Hz
static const int WhistleRangeUpperFrequencyRef = 3500;  // Hz
static_assert(WhistleRangeUpperFrequencyRef <= UsingRangeUpperFrequencyRef, "");
//@}

/// Whistle range values
//@{
static const int WhistleRangeLowerBinIndex = WhistleRangeLowerFrequencyRef * SignalSize / SampleRate;
static const int WhistleRangeUpperBinIndex = WhistleRangeUpperFrequencyRef * SignalSize / SampleRate;
static const int BinsPerWhistleRange = WhistleRangeUpperBinIndex - WhistleRangeLowerBinIndex + 1;
static_assert(BinsPerWhistleRange, "");
inline double whistleRangeLowerFrequency() { return hzPerBin() * WhistleRangeLowerBinIndex; }
inline double whistleRangeUpperFrequency() { return hzPerBin() * WhistleRangeUpperBinIndex; }
inline double hzPerWhistleRange() { return hzPerBin() * BinsPerWhistleRange; }
//@}

#define _GLOBAL_SPECIFY_MSECS_FOR_PROCESSING_STEP
#ifdef GLOBAL_SPECIFY_MSECS_FOR_PROCESSING_STEP
/// Processing step reference parameter
/// HwdPlugin v0.5.1: MsecsPerProcessingStepRef = 15
static const int MsecsPerProcessingStepRef = 15;//25;

/// Returns count of samples per processing step, i.e. a distance between two STFT frames
static const int SamplesPerProcessingStep = SampleRate * MsecsPerProcessingStepRef / 1000;
#else
static const int SamplesPerProcessingStep = SignalSize;
#endif

/// Returns processing step duration in msecs.
inline double msecsPerProcessingStep() { return double(SamplesPerProcessingStep) / SampleRate * 1000; }

}  // HwdLib::
}  // Akon::


/* ==TRASH==
*/
