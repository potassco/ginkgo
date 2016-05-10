#ifndef __FEEDBACK_LOOP__PRODUCTION__EVENT_MINIMIZED_H
#define __FEEDBACK_LOOP__PRODUCTION__EVENT_MINIMIZED_H

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
// EventMinimized
//
////////////////////////////////////////////////////////////////////////////////////////////////////

struct EventMinimized
{
	static EventMinimized fromJSON(const Json::Value &json);
	Json::Value toJSON() const;

	size_t removedLiterals;
	size_t remainingLiterals;
	size_t decreasedDegree;
	size_t finalDegree;
	size_t requiredTests;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

}
}
}

#endif
