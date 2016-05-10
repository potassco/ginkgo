#ifndef __FEEDBACK_LOOP__PRODUCTION__EVENT_FEEDBACK_EXTRACTED_H
#define __FEEDBACK_LOOP__PRODUCTION__EVENT_FEEDBACK_EXTRACTED_H

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
// EventFeedbackExtracted
//
////////////////////////////////////////////////////////////////////////////////////////////////////

struct EventFeedbackExtracted
{
	static EventFeedbackExtracted fromJSON(const Json::Value &json);
	Json::Value toJSON() const;

	enum class Mode
	{
		Unknown,
		StartOver,
		Resume
	};

	Mode mode;
	double duration;
	size_t requestedConstraints;
	size_t extractedConstraints;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

std::ostream &operator<<(std::ostream &ostream, const EventFeedbackExtracted::Mode &mode);
std::istream &operator>>(std::istream &istream, EventFeedbackExtracted::Mode &mode);

////////////////////////////////////////////////////////////////////////////////////////////////////

}
}
}

#endif
