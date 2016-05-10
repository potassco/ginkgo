#include <ginkgo/feedback-loop/production/FluentClosureUsage.h>

#include <iostream>
#include <boost/bimap.hpp>
#include <boost/assign.hpp>

namespace ginkgo
{
namespace feedbackLoop
{
namespace production
{

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// FluentClosureUsage
//
////////////////////////////////////////////////////////////////////////////////////////////////////

using FluentClosureUsageNames = boost::bimap<FluentClosureUsage, std::string>;
static FluentClosureUsageNames fluentClosureUsageNames = boost::assign::list_of<FluentClosureUsageNames::relation>
	(FluentClosureUsage::NoFluentClosure, "NoFluentClosure")
	(FluentClosureUsage::UseFluentClosure, "UseFluentClosure");

////////////////////////////////////////////////////////////////////////////////////////////////////

std::ostream &operator<<(std::ostream &ostream, const FluentClosureUsage &fluentClosureUsage)
{
	const auto match = fluentClosureUsageNames.left.find(fluentClosureUsage);

	if (match == fluentClosureUsageNames.left.end())
		return (ostream << "Unknown");

	return (ostream << (*match).second);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::istream &operator>>(std::istream &istream, FluentClosureUsage &fluentClosureUsage)
{
	std::string fluentClosureUsageName;
	istream >> fluentClosureUsageName;

	const auto match = fluentClosureUsageNames.right.find(fluentClosureUsageName);

	if (match == fluentClosureUsageNames.right.end())
		fluentClosureUsage = FluentClosureUsage::Unknown;
	else
		fluentClosureUsage = (*match).second;

	return istream;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

}
}
}
