#include <ginkgo/feedback-loop/production/EventConstraintLearned.h>

#include <iostream>

namespace ginkgo
{
namespace feedbackLoop
{
namespace production
{

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// EventConstraintLearned
//
////////////////////////////////////////////////////////////////////////////////////////////////////

EventConstraintLearned EventConstraintLearned::fromJSON(const Json::Value &json)
{
	EventConstraintLearned result;

	result.degree = json["Degree"].asUInt64();
	result.literals = json["Literals"].asUInt64();
	result.totalLearnedConstraints = json["TotalLearnedConstraints"].asUInt64();

	return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Json::Value EventConstraintLearned::toJSON() const
{
	Json::Value result;

	result["Degree"] = static_cast<Json::UInt64>(degree);
	result["Literals"] = static_cast<Json::UInt64>(literals);
	result["TotalLearnedConstraints"] = static_cast<Json::UInt64>(totalLearnedConstraints);

	return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

}
}
}
