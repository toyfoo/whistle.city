/// \file
/// \brief ofxIntegratedWhistleDetector class
/// \author Alexander Konin
/// \date 08.Oct.2014

#include "ofxIntegratedWhistleDetector.h"
#include <ofUtils.h>

// ofxIntegratedWhistleDetector::Whistle

std::string ofxIntegratedWhistleDetector::Whistle::toString() const
{
	if (isNull())
		return std::string("Whistle: Null");

	return "Whistle: #" + ofToString(no()) +
		", Frequency (Hz): " + ofToString(frequency()) +
		", Certainty: " + ofToString(certainty()) +
		", Time (msecs): " + ofToString(msecsPerDuration());
}

// ofxIntegratedWhistleDetector

ofxIntegratedWhistleDetector::ofxIntegratedWhistleDetector() :
	detector_(&whistleEventHandler, this),
	lastWhistleNo_()
{
}

ofxIntegratedWhistleDetector::Whistle ofxIntegratedWhistleDetector::getNextWhistle()
{
	Poco::ScopedLock<Poco::FastMutex> lock(mutex_);

	if (whistles_.empty())
		return Whistle();

	ofxIntegratedWhistleDetector::Whistle result = whistles_.front();
	whistles_.pop_front();
	return result;
}

void ofxIntegratedWhistleDetector::whistleEventHandler(int /*totalProcessingSteps*/,
	double frequency, double certainty, int processingStepsPerDuration)
{
	// This handler is called from Detector's worker thread, so synchronization is needed. Here I
	// simply put whistle info to deque that will be later printed on the form (in draw() method).

	Poco::ScopedLock<Poco::FastMutex> lock(mutex_);

	const ofxIntegratedWhistleDetector::Whistle whistle(++lastWhistleNo_,
		frequency, certainty, processingStepsPerDuration * detector_.msecsPerProcessingStep());

	if (MaxWhistles == whistles_.size())
		whistles_.pop_front();
	whistles_.push_back(whistle);
}


/* ==TRASH==
*/
