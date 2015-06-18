/// \file
/// \brief DyWaObertoneDetector class
/// \author Alexander Konin
/// \date 22.Sep.2014

#include "DyWaObertoneDetector.h"
#include "Global.h"
#include <dywapitchtrack.h>
#include <assert.h>

namespace Akon {
namespace HwdLib {

/*!
	\class DyWaObertoneDetector
	\brief
*/

// DyWaObertoneDetector

DyWaObertoneDetector::DyWaObertoneDetector(int sampleRate, int signalSize) :
	sampleRate_(sampleRate),
	signalSize_(signalSize),
	pitchTracker_(new _dywapitchtracker)
{
	static const int MinimalPitchFrequency = 130;  // needs 1024 samples
	const int minSampleRequired = dywapitch_neededsamplecount(MinimalPitchFrequency);
	assert(signalSize >= minSampleRequired);
	(void)minSampleRequired;

	dywapitch_inittracking(&*pitchTracker_);
}

DyWaObertoneDetector::~DyWaObertoneDetector()
{
}

double DyWaObertoneDetector::operator()(const double* signal)
{
	const double pitch = dywapitch_computepitch(&*pitchTracker_, const_cast<double*>(signal),
		0, signalSize_);
	if (0.0 == pitch) return VoidBin;

	const double pitchScale = double(sampleRate_) / 44100.0;
	const double hzPerBin = double(sampleRate_) / signalSize_;

	return pitch * pitchScale / hzPerBin;
}

}  // HwdLib::
}  // Akon::


/* ==TRASH==
*/
