/// \file
/// \brief WhistleDetector class
/// \author Alexander Konin
/// \date 08.Oct.2014

#pragma once

#include "@libhdr.h"
#include <deque>

#if defined(_MSC_VER)
	#pragma warning(push)
	#pragma warning(disable : 4251)  // dll-interface
#endif

namespace Akon {
namespace HwdLib {

// WhistleDetector

class HWDLIB_EXPORT WhistleDetector
{
	WhistleDetector(const WhistleDetector&);
	WhistleDetector& operator=(const WhistleDetector&);

public:
	explicit WhistleDetector(int binsPerWhistleRange);

	int powerThresholdPercent() const { return powerThresholdPercent_; }
	void setPowerThresholdPercent(int value);

	int stepsPerWhistleDuration() const { return stepsPerWhistleDuration_; }
	void setStepsPerWhistleDuration(int value);

	int binsPerFrequencyDeviation() const { return binsPerFrequencyDeviation_; }
	void setBinsPerFrequencyDeviation(int value);

	double operator()(double bin, double binPowerRatio, double& whistleCertainty,
		int& processingStepsPerWhistleDuration);

//Q_SIGNALS:
//	void powerThresholdPercentChanged(int value);
//	void stepsPerWhistleDurationChanged(int value);
//	void binsPerFrequencyDeviationChanged(int value);

private:
	int binsPerWhistleRange_;
	int powerThresholdPercent_;
	int stepsPerWhistleDuration_;
	int binsPerFrequencyDeviation_;  // per step
	std::deque<double> bins_;
	std::deque<double> certainties_;
	static double getAverageValueWith(const std::deque<double>& list, double value);
};

}  // HwdLib::
}  // Akon::

#if defined(_MSC_VER)
	#pragma warning(pop)
#endif


/* ==TRASH==
*/

/*
/// \file
/// \brief WhistleDetector class
/// \author Alexander Konin
/// \date 20.Jun.2014

#pragma once

#include "@libhdr.h"
#include <deque>

#if defined(_MSC_VER)
	#pragma warning(push)
	#pragma warning(disable : 4251)  // dll-interface
#endif

namespace Akon {
namespace HwdLib {

// WhistleDetector

class HWDLIB_EXPORT WhistleDetector
{
	WhistleDetector(const WhistleDetector&);
	WhistleDetector& operator=(const WhistleDetector&);

public:
	explicit WhistleDetector(int binsPerLookingRange);

	int powerThresholdPercent() const { return powerThresholdPercent_; }
	void setPowerThresholdPercent(int value);

	int stepsPerWhistleDuration() const { return stepsPerWhistleDuration_; }
	void setStepsPerWhistleDuration(int value);

	int binsPerFrequencyDeviation() const { return binsPerFrequencyDeviation_; }
	void setBinsPerFrequencyDeviation(int value);

	int operator()(int bin, double binPowerRatio, double& whistleCertainty);

//Q_SIGNALS:
//	void powerThresholdPercentChanged(int value);
//	void stepsPerWhistleDurationChanged(int value);
//	void binsPerFrequencyDeviationChanged(int value);

private:
	int binsPerLookingRange_;
	int powerThresholdPercent_;
	int stepsPerWhistleDuration_;
	int binsPerFrequencyDeviation_;  // per step
	std::deque<int> bins_;
	std::deque<double> certainties_;
};

}  // HwdLib::
}  // Akon::

#if defined(_MSC_VER)
	#pragma warning(pop)
#endif
*/
