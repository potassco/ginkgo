#ifndef __FEEDBACK_LOOP__PRODUCTION__EVENT_CONSTRAINTS_REMOVED_H
#define __FEEDBACK_LOOP__PRODUCTION__EVENT_CONSTRAINTS_REMOVED_H

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
// EventConstraintsRemoved
//
////////////////////////////////////////////////////////////////////////////////////////////////////

struct EventConstraintsRemoved
{
	static EventConstraintsRemoved fromJSON(const Json::Value &json);
	Json::Value toJSON() const;

	enum class Source
	{
		Unknown,
		Feedback,
		LearnedConstraints
	};

	enum class Reason
	{
		Unknown,
		ContainsTerminalLiteral,
		Subsumed,
		DegreeTooHigh,
		ContainsTooManyLiterals
	};

	Source source;
	Reason reason;
	size_t removedConstraints;
	size_t remainingConstraints;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

std::ostream &operator<<(std::ostream &ostream, const EventConstraintsRemoved::Source &source);
std::istream &operator>>(std::istream &istream, EventConstraintsRemoved::Source &source);
std::ostream &operator<<(std::ostream &ostream, const EventConstraintsRemoved::Reason &reason);
std::istream &operator>>(std::istream &istream, EventConstraintsRemoved::Reason &reason);

////////////////////////////////////////////////////////////////////////////////////////////////////

}
}
}

#endif
