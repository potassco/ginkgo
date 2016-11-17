#ifndef __FEEDBACK_LOOP__PRODUCTION__EVENT_CANDIDATE_TESTED_H
#define __FEEDBACK_LOOP__PRODUCTION__EVENT_CANDIDATE_TESTED_H

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
// EventCandidateTested
//
////////////////////////////////////////////////////////////////////////////////////////////////////

struct EventCandidateTested
{
	static EventCandidateTested fromJSON(const Json::Value &json);
	Json::Value toJSON() const;

	enum class Purpose
	{
		Unknown,
		Prove,
		Minimize
	};

	ProofType proofType;
	Purpose purpose;
	size_t candidateDegree;
	size_t candidateLiterals;
	ProofResult proofResult;
	double groundingTime;
	Json::Value claspJSONOutput;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

std::ostream &operator<<(std::ostream &ostream, const EventCandidateTested::Purpose &purpose);
std::istream &operator>>(std::istream &istream, EventCandidateTested::Purpose &purpose);

////////////////////////////////////////////////////////////////////////////////////////////////////

}
}
}

#endif
