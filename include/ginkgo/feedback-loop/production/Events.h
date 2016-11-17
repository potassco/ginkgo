#ifndef __FEEDBACK_LOOP__PRODUCTION__EVENTS_H
#define __FEEDBACK_LOOP__PRODUCTION__EVENTS_H

#include <iosfwd>
#include <vector>
#include <tuple>
#include <chrono>

#include <json/value.h>

#include <ginkgo/feedback-loop/production/ProofResult.h>
#include <ginkgo/feedback-loop/production/ProofType.h>
#include <ginkgo/feedback-loop/production/Environment.h>
#include <ginkgo/feedback-loop/production/Configuration.h>

#include <ginkgo/feedback-loop/production/EventFeedbackExtracted.h>
#include <ginkgo/feedback-loop/production/EventConstraintsRemoved.h>
#include <ginkgo/feedback-loop/production/EventMinimized.h>
#include <ginkgo/feedback-loop/production/EventCandidateTested.h>
#include <ginkgo/feedback-loop/production/EventConstraintLearned.h>
#include <ginkgo/feedback-loop/production/EventFinished.h>

namespace ginkgo
{
namespace feedbackLoop
{
namespace production
{

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Events
//
////////////////////////////////////////////////////////////////////////////////////////////////////

class Events
{
	public:
		using TimeStamp = double;

		template <typename T>
		using Timed = std::tuple<TimeStamp, T>;

	public:
		static Events fromJSON(const Json::Value &json);
		Json::Value toJSON() const;

		void startTimer();

		void notifyFeedbackExtracted(EventFeedbackExtracted event);
		void notifyConstraintsRemoved(const EventConstraintsRemoved &event);
		void notifyMinimized(const EventMinimized &event);
		void notifyCandidateTested(const EventCandidateTested &event);
		void notifyConstraintLearned(const EventConstraintLearned &event);
		void notifyFinished(const EventFinished &event);

		const std::vector<Timed<EventFeedbackExtracted>> &eventsFeedbackExtracted() const;
		const std::vector<Timed<EventConstraintsRemoved>> &eventsConstraintsRemoved() const;
		const std::vector<Timed<EventMinimized>> &eventsMinimized() const;
		const std::vector<Timed<EventCandidateTested>> &eventsCandidateTested() const;
		const std::vector<Timed<EventConstraintLearned>> &eventsConstraintLearned() const;
		const Timed<EventFinished> &eventFinished() const;

	private:
		TimeStamp time() const;

		std::vector<Timed<EventFeedbackExtracted>> m_eventsFeedbackExtracted;
		std::vector<Timed<EventConstraintsRemoved>> m_eventsConstraintsRemoved;
		std::vector<Timed<EventMinimized>> m_eventsMinimized;
		std::vector<Timed<EventCandidateTested>> m_eventsCandidateTested;
		std::vector<Timed<EventConstraintLearned>> m_eventsConstraintLearned;
		Timed<EventFinished> m_eventFinished;

		std::chrono::time_point<std::chrono::high_resolution_clock> m_startTime;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

}
}
}

#endif
