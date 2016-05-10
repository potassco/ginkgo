#include <ginkgo/feedback-loop/production/EventMinimized.h>

#include <iostream>

namespace ginkgo
{
namespace feedbackLoop
{
namespace production
{

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// EventMinimized
//
////////////////////////////////////////////////////////////////////////////////////////////////////

EventMinimized EventMinimized::fromJSON(const Json::Value &json)
{
	EventMinimized result;

	result.removedLiterals = json["RemovedLiterals"].asUInt64();
	result.remainingLiterals = json["RemainingLiterals"].asUInt64();
	result.decreasedDegree = json["DecreasedDegree"].asUInt64();
	result.finalDegree = json["FinalDegree"].asUInt64();
	result.requiredTests = json["RequiredTests"].asUInt64();

	return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Json::Value EventMinimized::toJSON() const
{
	Json::Value result;

	result["RemovedLiterals"] = static_cast<Json::UInt64>(removedLiterals);
	result["RemainingLiterals"] = static_cast<Json::UInt64>(remainingLiterals);
	result["DecreasedDegree"] = static_cast<Json::UInt64>(decreasedDegree);
	result["FinalDegree"] = static_cast<Json::UInt64>(finalDegree);
	result["RequiredTests"] = static_cast<Json::UInt64>(requiredTests);

	return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

}
}
}
