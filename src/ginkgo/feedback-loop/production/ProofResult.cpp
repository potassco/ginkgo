#include <ginkgo/feedback-loop/production/ProofResult.h>

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
// ProofResult
//
////////////////////////////////////////////////////////////////////////////////////////////////////

using ProofResultNames = boost::bimap<ProofResult, std::string>;
static ProofResultNames proofResultNames = boost::assign::list_of<ProofResultNames::relation>
	(ProofResult::Proven, "Proven")
	(ProofResult::Unproven, "Unproven")
	(ProofResult::GroundingTimeout, "GroundingTimeout")
	(ProofResult::SolvingTimeout, "SolvingTimeout");

////////////////////////////////////////////////////////////////////////////////////////////////////

std::ostream &operator<<(std::ostream &ostream, const ProofResult &proofResult)
{
	const auto match = proofResultNames.left.find(proofResult);

	if (match == proofResultNames.left.end())
		return (ostream << "Unknown");

	return (ostream << (*match).second);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::istream &operator>>(std::istream &istream, ProofResult &proofResult)
{
	std::string proofResultName;
	istream >> proofResultName;

	const auto match = proofResultNames.right.find(proofResultName);

	if (match == proofResultNames.right.end())
		proofResult = ProofResult::Unknown;
	else
		proofResult = (*match).second;

	return istream;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

}
}
}
