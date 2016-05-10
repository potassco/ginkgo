#ifndef __FEEDBACK_LOOP__PRODUCTION__EVENT_HYPOTHESIS_TESTED_H
#define __FEEDBACK_LOOP__PRODUCTION__EVENT_HYPOTHESIS_TESTED_H

#include <iosfwd>
#include <json/value.h>

#include <ginkgo/feedback-loop/production/ProofType.h>
#include <ginkgo/feedback-loop/production/ProofResult.h>

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

struct EventHypothesisTested
{
	static EventHypothesisTested fromJSON(const Json::Value &json);
	Json::Value toJSON() const;

	enum class Purpose
	{
		Unknown,
		Prove,
		Minimize
	};

	ProofType proofType;
	Purpose purpose;
	size_t hypothesisDegree;
	size_t hypothesisLiterals;
	ProofResult proofResult;
	double groundingTime;
	Json::Value claspJSONOutput;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

std::ostream &operator<<(std::ostream &ostream, const EventHypothesisTested::Purpose &purpose);
std::istream &operator>>(std::istream &istream, EventHypothesisTested::Purpose &purpose);

////////////////////////////////////////////////////////////////////////////////////////////////////

}
}
}

#endif
