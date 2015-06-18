/// \file
/// \brief ofxWhistleSequenceDetector class
/// \author Alexander Konin
/// \date 04.Sep.2014

#pragma once

#include <string>

// ofxWhistleSequenceDetector

class ofxWhistleSequenceDetector
{
	ofxWhistleSequenceDetector(const ofxWhistleSequenceDetector&);
	ofxWhistleSequenceDetector& operator=(const ofxWhistleSequenceDetector&);

public:
	explicit ofxWhistleSequenceDetector(int whistleCountInSequence = 4, int msecsPerStateGap = 1000,
		int msecsPerStateTimeout = 10 * 1000, int msecsPerFinalStateTimeout = 60 * 1000);

	int whistleCountInSequence() const { return whistleCountInSequence_; }
	void setWhistleCountInSequence(int value);

	int msecsPerStateGap() const { return msecsPerStateGap_; }
	void setMsecsPerStateGap(int value);

	int msecsPerStateTimeout() const { return msecsPerStateTimeout_; }
	void setMsecsPerStateTimeout(int value);

	int msecsPerFinalStateTimeout() const { return msecsPerFinalStateTimeout_; }
	void setMsecsPerFinalStateTimeout(int value);

	void reset();

	/// Describes state transition
	class Transition
	{
		friend class ofxWhistleSequenceDetector;

	public:
		/// Returns sequence ID to which this transition belongs.
		int sequenceId() const { return sequenceId_;}
		/// Returns initial state, i.e. 0. The initial state has 0% certainty.
		int initial() const { return 0; }
		/// Returns final state, i.e. max allowed count of whistles in the whistle sequence.
		/// The final state has 100% certainty.
		int final() const { return final_; }

		/// Returns previous state.
		int from() const { return from_; }
		/// Returns current state.
		int to() const { return to_; }
		/// Returns a certainty percent in accordance with the current state. Range: [0..100].
		int certaintyPercent() const { return 100 * to() / final(); }

		/// Returns flag indicating the current state is initial state, at this the certainty is 0%.
		bool isToInitial() const { return initial() == to(); }
		/// Returns flag indicating the current state is final state, at this the certainty is 100%.
		bool isToFinal() const { return final() == to(); }
		/// Returns flag indicating the previous state is initial state.
		bool isFromInitial() const { return initial() == from(); }
		/// Returns flag indicating the previous state is final state.
		bool isFromFinal() const { return final() == from(); }

		/// Returns flag indicating a transition to a new state.
		bool hasTransition() const { return from() != to(); }

		/// Returns object representation as string info.
		std::string toString() const;

	private:
		int sequenceId_;
		int final_;
		int from_;
		int to_;

		Transition(int sequenceId, int final, int from, int to) :
			sequenceId_(sequenceId),
			final_(final),
			from_(from),
			to_(to)
		{
		}
	};

	Transition update(bool realWhistle);

private:
	int whistleCountInSequence_;

	int msecsPerStateGap_;
	int msecsPerStateTimeout_;
	int msecsPerFinalStateTimeout_;

	int state_;
	int currentTime_;

	int sequenceId_;
};


/* ==TRASH==
*/
