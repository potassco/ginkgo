#include <ginkgo/feedback-loop/production/TestingPolicy.h>

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
// TestingPolicy
//
////////////////////////////////////////////////////////////////////////////////////////////////////

using TestingPolicyNames = boost::bimap<TestingPolicy, std::string>;
static TestingPolicyNames testingPolicyNames = boost::assign::list_of<TestingPolicyNames::relation>
	(TestingPolicy::FindFirst, "FindFirst")
	(TestingPolicy::TestAll, "TestAll");

////////////////////////////////////////////////////////////////////////////////////////////////////

std::ostream &operator<<(std::ostream &ostream, const TestingPolicy &testingPolicy)
{
	const auto match = testingPolicyNames.left.find(testingPolicy);

	if (match == testingPolicyNames.left.end())
		return (ostream << "Unknown");

	return (ostream << (*match).second);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::istream &operator>>(std::istream &istream, TestingPolicy &testingPolicy)
{
	std::string testingPolicyName;
	istream >> testingPolicyName;

	const auto match = testingPolicyNames.right.find(testingPolicyName);

	if (match == testingPolicyNames.right.end())
		testingPolicy = TestingPolicy::Unknown;
	else
		testingPolicy = (*match).second;

	return istream;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

}
}
}
