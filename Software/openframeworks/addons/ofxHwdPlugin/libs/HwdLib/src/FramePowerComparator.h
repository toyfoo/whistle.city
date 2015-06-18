/// \file
/// \brief FramePowerComparator class
/// \author Alexander Konin
/// \date 08.Oct.2014

#pragma once

#include "@libhdr.h"

class QSettings;

namespace Akon {
namespace HwdLib {

// FramePowerComparator

class HWDLIB_EXPORT FramePowerComparator
{
	FramePowerComparator(const FramePowerComparator&);
	FramePowerComparator& operator=(const FramePowerComparator&);

public:
	explicit FramePowerComparator(int binsPerUsingRange, float threshold = 0.001f);

	float threshold() const { return threshold_; }
	void setThreshold(float value);

	bool operator()(const double* usingRangePowerSpectrum, double& power);

//Q_SIGNALS:
//	void thresholdChanged(float value);

private:
	int binsPerUsingRange_;
	float threshold_;  // float to be atomic on 32 bits
};

}  // HwdLib::
}  // Akon::


/* ==TRASH==
*/
