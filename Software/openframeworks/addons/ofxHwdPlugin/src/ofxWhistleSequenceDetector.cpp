/// \file
/// \brief ofxWhistleSequenceDetector class
/// \author Alexander Konin
/// \date 04.Sep.2014

#include "ofxWhistleSequenceDetector.h"
#include <ofUtils.h>

/*
// Utils

static clock_t msecsToClocks(int msecs)
{
	return static_cast<long long>(msecs) * CLOCKS_PER_SEC / 1000;  // long long: to prevent overflow
}

static int clocksToMsecs(clock_t clocks)
{
	return static_cast<long long>(clocks) * 1000 / CLOCKS_PER_SEC;  // long long: to prevent overflow
}
*/

// ofxWhistleSequenceDetector::Transition

std::string ofxWhistleSequenceDetector::Transition::toString() const
{
	return "Transition: Seq.#" + ofToString(sequenceId()) +
		", " + ofToString(from()) + "->" + ofToString(to()) +
		", Certainty: " + ofToString(certaintyPercent()) + "%";
}

// ofxWhistleSequenceDetector

ofxWhistleSequenceDetector::ofxWhistleSequenceDetector(int whistleCountInSequence,
	int msecsPerStateGap, int msecsPerStateTimeout, int msecsPerFinalStateTimeout) :
	whistleCountInSequence_(whistleCountInSequence),

	msecsPerStateGap_(msecsPerStateGap),
	msecsPerStateTimeout_(msecsPerStateTimeout),
	msecsPerFinalStateTimeout_(msecsPerFinalStateTimeout),

	state_(0),
	currentTime_(ofGetSystemTime() - msecsPerStateTimeout_),  // looks like a real whistle in the past

	sequenceId_(0)
{
}

void ofxWhistleSequenceDetector::setWhistleCountInSequence(int value)
{
	value = std::max(value, 1);

	if (whistleCountInSequence_ == value) return;
	reset();
	whistleCountInSequence_ = value;
}

void ofxWhistleSequenceDetector::setMsecsPerStateGap(int value)
{
	value = std::max(value, 0);

	if (msecsPerStateGap_ == value) return;
	msecsPerStateGap_ = value;

//	if (msecsPerStateTimeout_ < msecsPerStateGap_)
//		msecsPerStateTimeout_ = msecsPerStateGap_;
//
//	if (msecsPerFinalStateTimeout_ < msecsPerStateGap_)
//		msecsPerFinalStateTimeout_ = msecsPerStateGap_;
}

void ofxWhistleSequenceDetector::setMsecsPerStateTimeout(int value)
{
	value = std::max(value, 0);

	if (msecsPerStateTimeout_ == value) return;
	msecsPerStateTimeout_ = value;
//	if (msecsPerStateTimeout_ < msecsPerStateGap_)
//		msecsPerStateGap_ = msecsPerStateTimeout_;
}

void ofxWhistleSequenceDetector::setMsecsPerFinalStateTimeout(int value)
{
	value = std::max(value, 0);

	if (msecsPerFinalStateTimeout_ == value) return;
	msecsPerFinalStateTimeout_ = value;

//	if (msecsPerFinalStateTimeout_ < msecsPerStateGap_)
//		msecsPerStateGap_ = msecsPerFinalStateTimeout_;
}

void ofxWhistleSequenceDetector::reset()
{
	state_ = 0;
}

ofxWhistleSequenceDetector::Transition ofxWhistleSequenceDetector::update(bool realWhistle)
{
	const int currentTime = ofGetSystemTime();
	const int deltaTime = currentTime - currentTime_;
	const int lastState = state_;

	if ((whistleCountInSequence_ != state_ && deltaTime >= msecsPerStateTimeout_)
		|| (whistleCountInSequence_ == state_
			&& deltaTime >= msecsPerFinalStateTimeout_))
		state_ = realWhistle ? 1 : 0;
	else if (whistleCountInSequence_ != state_ && deltaTime >= msecsPerStateGap_) {
		if (realWhistle)
			state_ = whistleCountInSequence_ != state_ ? (state_ + 1) : 1;
	}

	if (realWhistle)
		currentTime_ = currentTime;

	if (1 == state_ && lastState != state_)
		++sequenceId_;

	return Transition(sequenceId_, whistleCountInSequence_, lastState, state_);
}


/* ==TRASH==
//ofxWhistleSequenceDetector::Transition ofxWhistleSequenceDetector::update(bool realWhistle)
//{
//	const clock_t currentClock = clock();
//	const clock_t clockDelta = currentClock - clock_;
//	const clock_t clockDeltaFromFinalState = currentClock - finalStateClock_;
//	const int lastState = state_;
//
//	// Patch: hold final state during msecsPerFinalStateTimeout_ in any case
//	if (whistleCountInSequence_ == state_) {
//		if (clockDeltaFromFinalState >= msecsPerFinalStateTimeout_) {
//			state_ = realWhistle ? 1 : 0;
//			++sequenceId_;
//		}
//
//		if (realWhistle)
//			clock_ = currentClock;
//
//		return Transition(sequenceId_, whistleCountInSequence_, lastState, state_);
//	}
//
//	if (clockDelta >= msecsPerStateTimeout_)
//		state_ = realWhistle ? 1 : 0;
//	else if (clockDelta >= msecsPerStateGap_) {
//		if (realWhistle)
//			state_ = state_ < whistleCountInSequence_ ? (state_ + 1) : 1;
//				// < (less): setWhistleCountInSequence() may change whistleCountInSequence_
//	}
//
//	if (realWhistle) {
//		clock_ = currentClock;
//		if (state_ >= whistleCountInSequence_)
//			finalStateClock_ = currentClock;
//	}
//
//	if (lastState >= whistleCountInSequence_ && lastState != state_)
//		++sequenceId_;
//
//	return Transition(sequenceId_, whistleCountInSequence_, lastState, state_);
//}

// src:
//ofxWhistleSequenceDetector::Transition ofxWhistleSequenceDetector::update(bool realWhistle)
//{
//	const clock_t currentClock = clock();
//	const clock_t clockDelta = currentClock - clock_;
//	const int lastState = state_;
//
//	if (clockDelta >= msecsPerStateTimeout_)
//		state_ = realWhistle ? 1 : 0;
//	else if (clockDelta >= msecsPerStateGap_) {
//		if (realWhistle)
//			state_ = state_ < whistleCountInSequence_ ? (state_ + 1) : 1;
//				// < (less): setWhistleCountInSequence() may change whistleCountInSequence_
//	}
//
//	if (realWhistle)
//		clock_ = currentClock;
//
//	if (lastState != state_)
//		++sequenceId_;
//
//	return Transition(sequenceId_, whistleCountInSequence_, lastState, state_);
//}

//	if (clockDelta < msecsPerStateGap_)
//		;  // do nothing
//	else if (clockDelta < msecsPerStateTimeout_) {
//		if (realWhistle)
//			state_ = state_ < whistleCountInSequence_ ? (state_ + 1) : 1;
//				// setWhistleCountInSequence() may change down
//	} else
//		state_ = realWhistle ? 1 : 0;

//			if (state_ >= whistleCountInSequence_)  // setWhistleCountInSequence() may change down
//				state_ = 0;
//			++state_;

//#include <time.h>
//#include <windows.h>
//	clock_t time1 = clock();
//	::Sleep(500);
//	clock_t time2 = clock();
//	clock_t delta = time2 - time1;

	const int lastState = state_;

	if (whistle.isNull()) {
		if (clockDelta >= msecsPerStateTimeout_)
			state_ = 0;
	} else {
		if (0 == state_)
			state_ = 1;
		else {
			if (clockDelta < msecsPerStateGap_);  // do nothing
			else if (clockDelta < msecsPerStateTimeout_)
				if ()
				++state_;
			else
				state_ = 0;
		}
		clock_ = currentClock;
	}

	return Transition(lastState, state_);

ofxWhistleSequenceDetector::~ofxWhistleSequenceDetector()
{
}
*/
