/// \file
/// \brief ofxIntegratedWhistleDetector class
/// \author Alexander Konin
/// \date 08.Apr.2015

#pragma once

#include <IntegratedWhistleDetector.h>  // #include <HwdLib/IntegratedWhistleDetector>
#include <Poco/Mutex.h>
#include <deque>
#include <string>

// ofxIntegratedWhistleDetector

class ofxIntegratedWhistleDetector
{
	ofxIntegratedWhistleDetector(const ofxIntegratedWhistleDetector&);
	ofxIntegratedWhistleDetector& operator=(const ofxIntegratedWhistleDetector&);

public:
	ofxIntegratedWhistleDetector();

	int hzPerWindow() const { return detector_.hzPerWindow(); }
	void setHzPerWindow(int value) { detector_.setHzPerWindow(value); }

	int whistlePowerThresholdPercent() const { return detector_.whistlePowerThresholdPercent(); }
	void setWhistlePowerThresholdPercent(int value) { detector_.setWhistlePowerThresholdPercent(value); }

	int msecsPerWhistleDuration() const { return detector_.msecsPerWhistleDuration(); }
	void setMsecsPerWhistleDuration(int value) { detector_.setMsecsPerWhistleDuration(value); }

	int hzPerWhistleFrequencyDeviation() const { return detector_.hzPerWhistleFrequencyDeviation(); }
	void setHzPerWhistleFrequencyDeviation(int value) { detector_.setHzPerWhistleFrequencyDeviation(value); }

	float framePowerThreshold() const { return detector_.framePowerThreshold(); }
	void setFramePowerThreshold(float value) { detector_.setFramePowerThreshold(value); }
	
	/// Returns flag indicating overtone detector usage. With overtone detector the algorithm is
	/// more strict reducing false positive rate, but less tolerant.
	bool useOvertoneDetector() const { return detector_.useOvertoneDetector(); }
	void setUseOvertoneDetector(bool value) { detector_.setUseOvertoneDetector(value); }

	/// Returns flag indicating opened/closed state.
	bool isOpened() const { return detector_.isOpened(); }

	/// Opens underlying audio input and starts processing input signal.
	/// throws RtAudioError if something wrong with underlying audio input (RtAudio)
	void open() { detector_.open(); }

	/// Stops processing and closes underlying audio input.
	void close() { detector_.close(); }

	/// This exception is setted if the internal worker thread has finished with an error;
	/// otherwise this is zero pointer. Check the exception after the thread has finished (at this
	/// isOpen() returns false). The exception remains valid until open() is called.
	/// \note This function exposes only internal worker thread exceptions. Exceptions from other
	/// places (for instance, from open()) are not covered by this function.
	const std::runtime_error* workerThreadException() const
	{
		return detector_.workerThreadException();
	}

	/// Describes whistle information
	class Whistle
	{
		friend class ofxIntegratedWhistleDetector;

	public:
		/// The Whistle class supports so called null semantics. The function returns flag
		/// indicating null state.
		bool isNull() const { return 0 == no_; }

		/// Returns ordinal whistle number starting from 1. Null object has zero value.
		int no() const { return no_; }

		/// Returns whistle frequency, Hz. Null object has zero value.
		double frequency() const { return frequency_; }

		/// Returns whistle certainty, value from range [0..1]. Null object has zero value.
		double certainty() const { return certainty_; }

		/// Returns whistle duration in milliseconds. Null object has zero value.
		int msecsPerDuration() const { return msecsPerDuration_; }

		/// Returns object representation as string info. Null object has empty string.
		std::string toString() const;

	private:
		int no_;
		double frequency_;
		double certainty_;
		int msecsPerDuration_;

		Whistle(int no = 0, double frequency = 0, double certainty = 0, int msecsPerDuration = 0) :
			no_(no),
			frequency_(frequency),
			certainty_(certainty),
			msecsPerDuration_(msecsPerDuration)
		{
		}
	};

	/// Returns next detected whistle if one is available or null whistle (Whistle::isNull()->true)
	/// instead. This fuction is intended for periodical calling. ofxIntegratedWhistleDetector
	/// internally maintains a queue of detected whistles, this function returns and removes
	/// a whistle from the queue in FIFO order.
	Whistle getNextWhistle();

private:
	Akon::HwdLib::IntegratedWhistleDetector detector_;
	Poco::FastMutex mutex_;

	static const size_t MaxWhistles = 1000;
	std::deque<Whistle> whistles_;
	int lastWhistleNo_;

	static void whistleEventHandler(Akon::HwdLib::IntegratedWhistleDetector* /*sender*/,
		int totalProcessingSteps, double frequency, double certainty,
		int processingStepsPerDuration, void* userData)
	{
		ofxIntegratedWhistleDetector* asThis = static_cast<ofxIntegratedWhistleDetector*>(userData);
		asThis->whistleEventHandler(totalProcessingSteps, frequency, certainty,
			processingStepsPerDuration);
	}
	void whistleEventHandler(int totalProcessingSteps, double frequency, double certainty,
		int processingStepsPerDuration);
};


/* ==TRASH==
*/
