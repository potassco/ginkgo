#include <ginkgo/feedback-loop/production/MinimizationStrategy.h>

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
// MinimizationStrategy
//
////////////////////////////////////////////////////////////////////////////////////////////////////

using MinimizationStrategyNames = boost::bimap<MinimizationStrategy, std::string>;
static MinimizationStrategyNames minimizationStrategyNames = boost::assign::list_of<MinimizationStrategyNames::relation>
	(MinimizationStrategy::NoMinimization, "NoMinimization")
	(MinimizationStrategy::SimpleMinimization, "SimpleMinimization")
	(MinimizationStrategy::LinearMinimization, "LinearMinimization");

////////////////////////////////////////////////////////////////////////////////////////////////////

std::ostream &operator<<(std::ostream &ostream, const MinimizationStrategy &minimizationStrategy)
{
	const auto match = minimizationStrategyNames.left.find(minimizationStrategy);

	if (match == minimizationStrategyNames.left.end())
		return (ostream << "Unknown");

	return (ostream << (*match).second);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::istream &operator>>(std::istream &istream, MinimizationStrategy &minimizationStrategy)
{
	std::string minimizationStrategyName;
	istream >> minimizationStrategyName;

	const auto match = minimizationStrategyNames.right.find(minimizationStrategyName);

	if (match == minimizationStrategyNames.right.end())
		minimizationStrategy = MinimizationStrategy::Unknown;
	else
		minimizationStrategy = (*match).second;

	return istream;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

}
}
}
