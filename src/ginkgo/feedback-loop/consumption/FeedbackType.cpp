#include <ginkgo/feedback-loop/consumption/FeedbackType.h>

#include <iostream>
#include <boost/bimap.hpp>
#include <boost/assign.hpp>

namespace ginkgo
{
namespace feedbackLoop
{
namespace consumption
{

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// FeedbackType
//
////////////////////////////////////////////////////////////////////////////////////////////////////

using FeedbackTypeNames = boost::bimap<FeedbackType, std::string>;
static FeedbackTypeNames feedbackTypeNames = boost::assign::list_of<FeedbackTypeNames::relation>
	(FeedbackType::Direct, "Direct")
	(FeedbackType::Generalized, "Generalized");

////////////////////////////////////////////////////////////////////////////////////////////////////

std::ostream &operator<<(std::ostream &ostream, const FeedbackType &feedbackType)
{
	const auto match = feedbackTypeNames.left.find(feedbackType);

	if (match == feedbackTypeNames.left.end())
		return (ostream << "Unknown");

	return (ostream << (*match).second);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::istream &operator>>(std::istream &istream, FeedbackType &feedbackType)
{
	std::string feedbackTypeName;
	istream >> feedbackTypeName;

	const auto match = feedbackTypeNames.right.find(feedbackTypeName);

	if (match == feedbackTypeNames.right.end())
		feedbackType = FeedbackType::Unknown;
	else
		feedbackType = (*match).second;

	return istream;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

}
}
}
