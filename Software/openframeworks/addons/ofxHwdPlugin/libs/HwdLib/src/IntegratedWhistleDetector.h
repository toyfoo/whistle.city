/// \file
/// \brief IntegratedWhistleDetector class
/// \author Alexander Konin
/// \date 08.Apr.2015

//##note: oF does not support partial C++11 (std-0x11), only C++11 or C++2003. So, this header
// exposes all required interface of HwdLib without std-0x11 (e.g. static_assert).

#pragma once

#include "@libhdr.h"

namespace std { class runtime_error; }

namespace Akon {
namespace HwdLib {

// IntegratedWhistleDetector

class IntegratedWhistleDetectorPrivate;
class HWDLIB_EXPORT IntegratedWhistleDetector
{
	IntegratedWhistleDetector(const IntegratedWhistleDetector&);
	IntegratedWhistleDetector& operator=(const IntegratedWhistleDetector&);

public:
	//##note: boost::signals here are welcome (as observer pattern)
	typedef void (*WhistleEventHandler)(IntegratedWhistleDetector* sender,
		int totalProcessingSteps, double frequency, double certainty,
		int processingStepsPerDuration, void* userData);

	explicit IntegratedWhistleDetector(WhistleEventHandler whistleEventHandler, void* userData = 0);
	~IntegratedWhistleDetector();

	// Global
	/// Returns processing step duration in msecs.
	static double msecsPerProcessingStep();

	// Algorithm's parameters
	int hzPerWindow() const;
	void setHzPerWindow(int value);

	int whistlePowerThresholdPercent() const;
	void setWhistlePowerThresholdPercent(int value);

	int msecsPerWhistleDuration() const;
	void setMsecsPerWhistleDuration(int value);

	int hzPerWhistleFrequencyDeviation() const;
	void setHzPerWhistleFrequencyDeviation(int value);

	float framePowerThreshold() const;
	void setFramePowerThreshold(float value);
	
	bool useOvertoneDetector() const;
	void setUseOvertoneDetector(bool value);

	// Signal input
	bool isOpened() const;
	void open();
	void close();

	const std::runtime_error* workerThreadException() const;

private:
	IntegratedWhistleDetectorPrivate* pimpl_;
};

}  // HwdLib::
}  // Akon::


/* ==TRASH==
*/
