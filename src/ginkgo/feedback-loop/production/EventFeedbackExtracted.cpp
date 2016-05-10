#include <ginkgo/feedback-loop/production/EventFeedbackExtracted.h>

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
// EventFeedbackExtracted
//
////////////////////////////////////////////////////////////////////////////////////////////////////

EventFeedbackExtracted EventFeedbackExtracted::fromJSON(const Json::Value &json)
{
	EventFeedbackExtracted result;

	result.mode = fromString<Mode>(json["Mode"].asString());
	result.duration = json["Duration"].asDouble();
	result.requestedConstraints = json["RequestedConstraints"].asUInt64();
	result.extractedConstraints = json["ExtractedConstraints"].asUInt64();

	return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Json::Value EventFeedbackExtracted::toJSON() const
{
	Json::Value result;

	result["Mode"] = toString(mode);
	result["Duration"] = duration;
	result["RequestedConstraints"] = static_cast<Json::UInt64>(requestedConstraints);
	result["ExtractedConstraints"] = static_cast<Json::UInt64>(extractedConstraints);

	return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

using ModeNames = boost::bimap<EventFeedbackExtracted::Mode, std::string>;
static ModeNames modeNames = boost::assign::list_of<ModeNames::relation>
	(EventFeedbackExtracted::Mode::StartOver, "StartOver")
	(EventFeedbackExtracted::Mode::Resume, "Resume");

////////////////////////////////////////////////////////////////////////////////////////////////////

std::ostream &operator<<(std::ostream &ostream, const EventFeedbackExtracted::Mode &mode)
{
	const auto match = modeNames.left.find(mode);

	if (match == modeNames.left.end())
		return (ostream << "Unknown");

	return (ostream << (*match).second);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::istream &operator>>(std::istream &istream, EventFeedbackExtracted::Mode &mode)
{
	std::string modeName;
	istream >> modeName;

	const auto match = modeNames.right.find(modeName);

	if (match == modeNames.right.end())
		mode = EventFeedbackExtracted::Mode::Unknown;
	else
		mode = (*match).second;

	return istream;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

}
}
}
