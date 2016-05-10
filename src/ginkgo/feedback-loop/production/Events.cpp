#include <ginkgo/feedback-loop/production/Events.h>

#include <iostream>
#include <json/json.h>

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

Events Events::fromJSON(const Json::Value &json)
{
	Events statistics;

	auto &eventsFeedbackExtracted = json["FeedbackExtracted"];

	std::for_each(eventsFeedbackExtracted.begin(), eventsFeedbackExtracted.end(),
		[&](const auto &jsonEvent)
		{
			const auto event = EventFeedbackExtracted::fromJSON(jsonEvent);
			const auto time = jsonEvent["Time"].asDouble();

			statistics.m_eventsFeedbackExtracted.emplace_back(std::make_tuple(time, event));
		});

	auto &eventsConstraintsRemoved = json["ConstraintsRemoved"];

	std::for_each(eventsConstraintsRemoved.begin(), eventsConstraintsRemoved.end(),
		[&](const auto &jsonEvent)
		{
			const auto event = EventConstraintsRemoved::fromJSON(jsonEvent);
			const auto time = jsonEvent["Time"].asDouble();

			statistics.m_eventsConstraintsRemoved.emplace_back(std::make_tuple(time, event));
		});

	auto &eventsMinimized = json["Minimized"];

	std::for_each(eventsMinimized.begin(), eventsMinimized.end(),
		[&](const auto &jsonEvent)
		{
			const auto event = EventMinimized::fromJSON(jsonEvent);
			const auto time = jsonEvent["Time"].asDouble();

			statistics.m_eventsMinimized.emplace_back(std::make_tuple(time, event));
		});

	auto &eventsHypothesisTested = json["HypothesisTested"];

	std::for_each(eventsHypothesisTested.begin(), eventsHypothesisTested.end(),
		[&](const auto &jsonEvent)
		{
			const auto event = EventHypothesisTested::fromJSON(jsonEvent);
			const auto time = jsonEvent["Time"].asDouble();

			statistics.m_eventsHypothesisTested.emplace_back(std::make_tuple(time, event));
		});

	auto &eventsConstraintLearned = json["ConstraintLearned"];

	std::for_each(eventsConstraintLearned.begin(), eventsConstraintLearned.end(),
		[&](const auto &jsonEvent)
		{
			const auto event = EventConstraintLearned::fromJSON(jsonEvent);
			const auto time = jsonEvent["Time"].asDouble();

			statistics.m_eventsConstraintLearned.emplace_back(std::make_tuple(time, event));
		});

	auto &jsonEventFinished = json["Finished"];
	const auto eventFinished = EventFinished::fromJSON(jsonEventFinished);
	const auto timeFinished = jsonEventFinished["Time"].asDouble();

	statistics.m_eventFinished = std::make_tuple(timeFinished, eventFinished);

	return statistics;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Json::Value Events::toJSON() const
{
	Json::Value json;

	json["FeedbackExtracted"] = Json::arrayValue;

	std::for_each(m_eventsFeedbackExtracted.cbegin(), m_eventsFeedbackExtracted.cend(),
		[&](const auto &event)
		{
			auto jsonEvent = std::get<1>(event).toJSON();
			jsonEvent["Time"] = std::get<0>(event);

			json["FeedbackExtracted"].append(jsonEvent);
		});

	json["ConstraintsRemoved"] = Json::arrayValue;

	std::for_each(m_eventsConstraintsRemoved.cbegin(), m_eventsConstraintsRemoved.cend(),
		[&](const auto &event)
		{
			auto jsonEvent = std::get<1>(event).toJSON();
			jsonEvent["Time"] = std::get<0>(event);

			json["ConstraintsRemoved"].append(jsonEvent);
		});

	json["Minimized"] = Json::arrayValue;

	std::for_each(m_eventsMinimized.cbegin(), m_eventsMinimized.cend(),
		[&](const auto &event)
		{
			auto jsonEvent = std::get<1>(event).toJSON();
			jsonEvent["Time"] = std::get<0>(event);

			json["Minimized"].append(jsonEvent);
		});

	json["HypothesisTested"] = Json::arrayValue;

	std::for_each(m_eventsHypothesisTested.cbegin(), m_eventsHypothesisTested.cend(),
		[&](const auto &event)
		{
			auto jsonEvent = std::get<1>(event).toJSON();
			jsonEvent["Time"] = std::get<0>(event);

			json["HypothesisTested"].append(jsonEvent);
		});

	json["ConstaintLearned"] = Json::arrayValue;

	std::for_each(m_eventsConstraintLearned.cbegin(), m_eventsConstraintLearned.cend(),
		[&](const auto &event)
		{
			auto jsonEvent = std::get<1>(event).toJSON();
			jsonEvent["Time"] = std::get<0>(event);

			json["ConstraintLearned"].append(jsonEvent);
		});

	json["Finished"] = std::get<1>(m_eventFinished).toJSON();
	json["Finished"]["Time"] = std::get<0>(m_eventFinished);

	return json;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Events::startTimer()
{
	m_startTime = std::chrono::high_resolution_clock::now();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Events::notifyFeedbackExtracted(EventFeedbackExtracted event)
{
	m_eventsFeedbackExtracted.emplace_back(std::make_tuple(time(), event));
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Events::notifyConstraintsRemoved(const EventConstraintsRemoved &event)
{
	m_eventsConstraintsRemoved.emplace_back(std::make_tuple(time(), event));
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Events::notifyMinimized(const EventMinimized &event)
{
	m_eventsMinimized.emplace_back(std::make_tuple(time(), event));
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Events::notifyHypothesisTested(const EventHypothesisTested &event)
{
	m_eventsHypothesisTested.emplace_back(std::make_tuple(time(), event));
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Events::notifyConstraintLearned(const EventConstraintLearned &event)
{
	m_eventsConstraintLearned.emplace_back(std::make_tuple(time(), event));
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Events::notifyFinished(const EventFinished &event)
{
	m_eventFinished = std::make_tuple(time(), event);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

const std::vector<Events::Timed<EventFeedbackExtracted>> &Events::eventsFeedbackExtracted() const
{
	return m_eventsFeedbackExtracted;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

const std::vector<Events::Timed<EventConstraintsRemoved>> &Events::eventsConstraintsRemoved() const
{
	return m_eventsConstraintsRemoved;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

const std::vector<Events::Timed<EventMinimized>> &Events::eventsMinimized() const
{
	return m_eventsMinimized;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

const std::vector<Events::Timed<EventHypothesisTested>> &Events::eventsHypothesisTested() const
{
	return m_eventsHypothesisTested;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

const std::vector<Events::Timed<EventConstraintLearned>> &Events::eventsConstraintLearned() const
{
	return m_eventsConstraintLearned;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

const Events::Timed<EventFinished> &Events::eventFinished() const
{
	return m_eventFinished;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Events::TimeStamp Events::time() const
{
	const auto now = std::chrono::high_resolution_clock::now();
	return std::chrono::duration<TimeStamp>(now - m_startTime).count();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

}
}
}
