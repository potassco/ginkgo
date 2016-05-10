#include <ginkgo/feedback-loop/production/EventFinished.h>

#include <iostream>

#include <boost/bimap.hpp>
#include <boost/assign.hpp>

#include <ginkgo/utils/Utils.h>

namespace ginkgo
{
namespace feedbackLoop
{
namespace production
{

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// EventFinished
//
////////////////////////////////////////////////////////////////////////////////////////////////////

EventFinished EventFinished::fromJSON(const Json::Value &json)
{
	EventFinished result;

	result.reason = fromString<Reason>(json["Reason"].asString());

	return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Json::Value EventFinished::toJSON() const
{
	Json::Value result;

	result["Reason"] = toString(reason);

	return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

using ReasonNames = boost::bimap<EventFinished::Reason, std::string>;
static ReasonNames reasonNames = boost::assign::list_of<ReasonNames::relation>
	(EventFinished::Reason::Done, "Done")
	(EventFinished::Reason::FeedbackEmpty, "FeedbackEmpty")
	(EventFinished::Reason::ExtractionTimeout, "ExtractionTimeout");

////////////////////////////////////////////////////////////////////////////////////////////////////

std::ostream &operator<<(std::ostream &ostream, const EventFinished::Reason &reason)
{
	const auto match = reasonNames.left.find(reason);

	if (match == reasonNames.left.end())
		return (ostream << "Unknown");

	return (ostream << (*match).second);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::istream &operator>>(std::istream &istream, EventFinished::Reason &reason)
{
	std::string reasonName;
	istream >> reasonName;

	const auto match = reasonNames.right.find(reasonName);

	if (match == reasonNames.right.end())
		reason = EventFinished::Reason::Unknown;
	else
		reason = (*match).second;

	return istream;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

}
}
}
