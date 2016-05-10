#include <ginkgo/feedback-loop/production/EventConstraintsRemoved.h>

#include <iostream>

#include <boost/bimap.hpp>
#include <boost/assign.hpp>

namespace ginkgo
{
namespace feedbackLoop
{
namespace production
{

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// EventConstraintsRemoved
//
////////////////////////////////////////////////////////////////////////////////////////////////////

EventConstraintsRemoved EventConstraintsRemoved::fromJSON(const Json::Value &json)
{
	EventConstraintsRemoved result;

	std::stringstream sourceStream(json["Source"].asString());
	std::stringstream reasonStream(json["Reason"].asString());
	sourceStream >> result.source;
	reasonStream >> result.reason;
	result.removedConstraints = json["RemovedConstraints"].asUInt64();
	result.remainingConstraints = json["RemainingConstraints"].asUInt64();

	return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Json::Value EventConstraintsRemoved::toJSON() const
{
	Json::Value result;

	std::stringstream sourceStream;
	std::stringstream reasonStream;
	sourceStream << source;
	reasonStream << reason;
	result["Source"] = sourceStream.str();
	result["Reason"] = reasonStream.str();
	result["RemovedConstraints"] = static_cast<Json::UInt64>(removedConstraints);
	result["RemainingConstraints"] = static_cast<Json::UInt64>(remainingConstraints);

	return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

using SourceNames = boost::bimap<EventConstraintsRemoved::Source, std::string>;
static SourceNames sourceNames = boost::assign::list_of<SourceNames::relation>
	(EventConstraintsRemoved::Source::Feedback, "Feedback")
	(EventConstraintsRemoved::Source::LearnedConstraints, "LearnedConstraints");

////////////////////////////////////////////////////////////////////////////////////////////////////

std::ostream &operator<<(std::ostream &ostream, const EventConstraintsRemoved::Source &source)
{
	const auto match = sourceNames.left.find(source);

	if (match == sourceNames.left.end())
		return (ostream << "Unknown");

	return (ostream << (*match).second);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::istream &operator>>(std::istream &istream, EventConstraintsRemoved::Source &source)
{
	std::string sourceName;
	istream >> sourceName;

	const auto match = sourceNames.right.find(sourceName);

	if (match == sourceNames.right.end())
		source = EventConstraintsRemoved::Source::Unknown;
	else
		source = (*match).second;

	return istream;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

using ReasonNames = boost::bimap<EventConstraintsRemoved::Reason, std::string>;
static ReasonNames reasonNames = boost::assign::list_of<ReasonNames::relation>
	(EventConstraintsRemoved::Reason::ContainsTerminalLiteral, "ContainsTerminalLiteral")
	(EventConstraintsRemoved::Reason::Subsumed, "Subsumed")
	(EventConstraintsRemoved::Reason::DegreeTooHigh, "DegreeTooHigh")
	(EventConstraintsRemoved::Reason::ContainsTooManyLiterals, "ContainsTooManyLiterals");

////////////////////////////////////////////////////////////////////////////////////////////////////

std::ostream &operator<<(std::ostream &ostream, const EventConstraintsRemoved::Reason &reason)
{
	const auto match = reasonNames.left.find(reason);

	if (match == reasonNames.left.end())
		return (ostream << "Unknown");

	return (ostream << (*match).second);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::istream &operator>>(std::istream &istream, EventConstraintsRemoved::Reason &reason)
{
	std::string reasonName;
	istream >> reasonName;

	const auto match = reasonNames.right.find(reasonName);

	if (match == reasonNames.right.end())
		reason = EventConstraintsRemoved::Reason::Unknown;
	else
		reason = (*match).second;

	return istream;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

}
}
}
