/// \file
/// \brief FramePowerComparator class
/// \author Alexander Konin
/// \date 08.Oct.2014

#include "FramePowerComparator.h"
#include "Utils.h"

namespace Akon {
namespace HwdLib {

/*!
	\class FramePowerComparator
	\brief
*/

// FramePowerComparator

FramePowerComparator::FramePowerComparator(int binsPerUsingRange, float threshold) :
	binsPerUsingRange_(binsPerUsingRange),
	threshold_(threshold)
{
}

void FramePowerComparator::setThreshold(float value)
{
	//value = qBound(0.0f, value, 1.0f);
	if (value < 0.0f)
		value = 0.0f;
	if (value > 1.0f)
		value = 1.0f;
	if (threshold_ == value) return;
	threshold_ = value;
	//Q_EMIT thresholdChanged(threshold_);
}

bool FramePowerComparator::operator()(const double* usingRangePowerSpectrum, double& power)
{
	// Params may change from other (user) thread, check that they change with atomic instruction
	// (mov/add m32/m64), i.e. they have size <= sizeof(void*)
	static_assert(sizeof(threshold_) <= sizeof(void*), "");

	//##note: A harmonic with amplitude = 1.0 on its period has energy 0.5. One harmonic signal with
	// amplitude = 1.0 is considered as max power signal, so max power frame has 0.5 value.

	power = sum(usingRangePowerSpectrum, binsPerUsingRange_) * 2.0;  // normalized
	return power >= threshold_;
}

}  // HwdLib::
}  // Akon::


/* ==TRASH==
*/
