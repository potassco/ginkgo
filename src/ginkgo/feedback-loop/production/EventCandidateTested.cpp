#include <ginkgo/feedback-loop/production/EventCandidateTested.h>

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
// EventCandidateTested
//
////////////////////////////////////////////////////////////////////////////////////////////////////

EventCandidateTested EventCandidateTested::fromJSON(const Json::Value &json)
{
	EventCandidateTested result;

	result.proofType = fromString<ProofType>(json["ProofType"].asString());
	result.purpose = fromString<Purpose>(json["Purpose"].asString());
	result.candidateDegree = json["CandidateDegree"].asUInt64();
	result.candidateLiterals = json["CandidateLiterals"].asUInt64();
	result.proofResult = fromString<ProofResult>(json["ProofResult"].asString());
	result.groundingTime = json["GroundingTime"].asDouble();
	result.claspJSONOutput = json["ClaspOutput"];

	return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Json::Value EventCandidateTested::toJSON() const
{
	Json::Value result;

	result["ProofType"] = toString(proofType);
	result["Purpose"] = toString(purpose);
	result["CandidateDegree"] = static_cast<Json::UInt64>(candidateDegree);
	result["CandidateLiterals"] = static_cast<Json::UInt64>(candidateLiterals);
	result["ProofResult"] = toString(proofResult);
	result["GroundingTime"] = groundingTime;
	result["ClaspOutput"] = claspJSONOutput;

	return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

using PurposeNames = boost::bimap<EventCandidateTested::Purpose, std::string>;
static PurposeNames purposeNames = boost::assign::list_of<PurposeNames::relation>
	(EventCandidateTested::Purpose::Prove, "Prove")
	(EventCandidateTested::Purpose::Minimize, "Minimize");

////////////////////////////////////////////////////////////////////////////////////////////////////

std::ostream &operator<<(std::ostream &ostream, const EventCandidateTested::Purpose &purpose)
{
	const auto match = purposeNames.left.find(purpose);

	if (match == purposeNames.left.end())
		return (ostream << "Unknown");

	return (ostream << (*match).second);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::istream &operator>>(std::istream &istream, EventCandidateTested::Purpose &purpose)
{
	std::string purposeName;
	istream >> purposeName;

	const auto match = purposeNames.right.find(purposeName);

	if (match == purposeNames.right.end())
		purpose = EventCandidateTested::Purpose::Unknown;
	else
		purpose = (*match).second;

	return istream;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

}
}
}
