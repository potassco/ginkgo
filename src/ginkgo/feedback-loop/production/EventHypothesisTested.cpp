#include <ginkgo/feedback-loop/production/EventHypothesisTested.h>

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
// EventHypothesisTested
//
////////////////////////////////////////////////////////////////////////////////////////////////////

EventHypothesisTested EventHypothesisTested::fromJSON(const Json::Value &json)
{
	EventHypothesisTested result;

	result.proofType = fromString<ProofType>(json["ProofType"].asString());
	result.purpose = fromString<Purpose>(json["Purpose"].asString());
	result.hypothesisDegree = json["HypothesisDegree"].asUInt64();
	result.hypothesisLiterals = json["HypothesisLiterals"].asUInt64();
	result.proofResult = fromString<ProofResult>(json["ProofResult"].asString());
	result.groundingTime = json["GroundingTime"].asDouble();
	result.claspJSONOutput = json["ClaspOutput"];

	return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Json::Value EventHypothesisTested::toJSON() const
{
	Json::Value result;

	result["ProofType"] = toString(proofType);
	result["Purpose"] = toString(purpose);
	result["HypothesisDegree"] = static_cast<Json::UInt64>(hypothesisDegree);
	result["HypothesisLiterals"] = static_cast<Json::UInt64>(hypothesisLiterals);
	result["ProofResult"] = toString(proofResult);
	result["GroundingTime"] = groundingTime;
	result["ClaspOutput"] = claspJSONOutput;

	return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

using PurposeNames = boost::bimap<EventHypothesisTested::Purpose, std::string>;
static PurposeNames purposeNames = boost::assign::list_of<PurposeNames::relation>
	(EventHypothesisTested::Purpose::Prove, "Prove")
	(EventHypothesisTested::Purpose::Minimize, "Minimize");

////////////////////////////////////////////////////////////////////////////////////////////////////

std::ostream &operator<<(std::ostream &ostream, const EventHypothesisTested::Purpose &purpose)
{
	const auto match = purposeNames.left.find(purpose);

	if (match == purposeNames.left.end())
		return (ostream << "Unknown");

	return (ostream << (*match).second);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::istream &operator>>(std::istream &istream, EventHypothesisTested::Purpose &purpose)
{
	std::string purposeName;
	istream >> purposeName;

	const auto match = purposeNames.right.find(purposeName);

	if (match == purposeNames.right.end())
		purpose = EventHypothesisTested::Purpose::Unknown;
	else
		purpose = (*match).second;

	return istream;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

}
}
}
