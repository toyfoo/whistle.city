/// \file
/// \brief IntegratedWhistleDetectorPrivate class
/// \author Alexander Konin
/// \date 08.Apr.2015

#pragma once

#include "DyWaObertoneDetector.h"
#include "FramePowerComparator.h"
#include "WindowSpikeDetector.h"
#include "WhistleDetector.h"
#include <Akon/Fftw/Fft.h>
#include <RtAudio.h>
#include <memory>
#include <stdexcept>

namespace Akon {
namespace HwdLib {

// IntegratedWhistleDetectorPrivate

#define USE_2_CONSEQUITIVE_OBERTONES
#define USE_WEAK_SPIKES_SHADOWING
class IntegratedWhistleDetectorPrivate
{
public:
	explicit IntegratedWhistleDetectorPrivate(IntegratedWhistleDetector& self,
		IntegratedWhistleDetector::WhistleEventHandler whistleEventHandler, void* userData);
	~IntegratedWhistleDetectorPrivate();

	IntegratedWhistleDetector& self_;

	IntegratedWhistleDetector::WhistleEventHandler whistleEventHandler_;
	void* userData_;

	RtAudio audio_;
	std::unique_ptr<std::runtime_error> workerThreadException_;

	std::unique_ptr<double[], Akon::Fftw::DataDeleter<double[]> > signalBuffer_;
	int signalBufferCurrentSize_;

	Akon::Fftw::Fft fft_;
	std::unique_ptr<fftw_complex[], Akon::Fftw::DataDeleter<fftw_complex[]> > fftOutput_;
	std::unique_ptr<double[], Akon::Fftw::DataDeleter<double[]> > powerSpectrum_;  // full range
	std::unique_ptr<double[], Akon::Fftw::DataDeleter<double[]> > magnitudes_;  // full range

	Akon::HwdLib::FramePowerComparator framePowerComparator_;
	Akon::HwdLib::WindowSpikeDetector windowSpikeDetector_;
	Akon::HwdLib::DyWaObertoneDetector dyWaObertoneDetector_;
	Akon::HwdLib::WhistleDetector whistleDetector_;

#ifdef USE_WEAK_SPIKES_SHADOWING
	double prevSpikeLevel_;
	double prevSpikeDurationCounter_;
#endif

#ifdef USE_2_CONSEQUITIVE_OBERTONES
	double prevObertoneBin_;
#endif
	
	bool useOvertoneDetector_;

	static int audioCallback(void* /*outputBuffer*/, void* inputBuffer, unsigned int nBufferFrames,
		double /*streamTime*/, RtAudioStreamStatus status, void* userData)
	{
		// use: boost_polymorphic_downcast here to be more strong
		IntegratedWhistleDetectorPrivate* asThis = static_cast<IntegratedWhistleDetectorPrivate*>(
			userData);
		return asThis->audioCallback(inputBuffer, nBufferFrames, status);
	}
	int audioCallback(void* inputBuffer, unsigned int nBufferFrames, RtAudioStreamStatus status);

	void processData(const double* data, int size);

	void patchSpikeBinIfObertones(double& spikeBin, double obertoneBin);
#ifdef USE_WEAK_SPIKES_SHADOWING
	void patchSpikeBinIfItsLevelIsTooSmall(double& spikeBin, double spikeLevel);
#endif
};

}  // HwdLib::
}  // Akon::


/* ==TRASH==
*/
