#ifndef __FEEDBACK_LOOP__PRODUCTION__EVENT_CONSTRAINT_LEARNED_H
#define __FEEDBACK_LOOP__PRODUCTION__EVENT_CONSTRAINT_LEARNED_H

#include <iosfwd>
#include <json/value.h>

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

struct EventConstraintLearned
{
	static EventConstraintLearned fromJSON(const Json::Value &json);
	Json::Value toJSON() const;

	size_t degree;
	size_t literals;
	size_t totalLearnedConstraints;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

}
}
}

#endif
