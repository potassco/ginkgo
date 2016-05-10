#ifndef __FEEDBACK_LOOP__PRODUCTION__EVENT_FINISHED_H
#define __FEEDBACK_LOOP__PRODUCTION__EVENT_FINISHED_H

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
// EventFinished
//
////////////////////////////////////////////////////////////////////////////////////////////////////

struct EventFinished
{
	static EventFinished fromJSON(const Json::Value &json);
	Json::Value toJSON() const;

	enum class Reason
	{
		Unknown,
		Done,
		FeedbackEmpty,
		ExtractionTimeout
	};

	Reason reason;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

std::ostream &operator<<(std::ostream &ostream, const EventFinished::Reason &reason);
std::istream &operator>>(std::istream &istream, EventFinished::Reason &reason);

////////////////////////////////////////////////////////////////////////////////////////////////////

}
}
}

#endif
