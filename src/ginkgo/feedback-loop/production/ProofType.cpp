#include <ginkgo/feedback-loop/production/ProofType.h>

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
// ProofType
//
////////////////////////////////////////////////////////////////////////////////////////////////////

using ProofTypeNames = boost::bimap<ProofType, std::string>;
static ProofTypeNames proofTypeNames = boost::assign::list_of<ProofTypeNames::relation>
	(ProofType::StateWise, "StateWise")
	(ProofType::InductiveBase, "InductiveBase")
	(ProofType::InductiveStep, "InductiveStep");

////////////////////////////////////////////////////////////////////////////////////////////////////

std::ostream &operator<<(std::ostream &ostream, const ProofType &proofType)
{
	const auto match = proofTypeNames.left.find(proofType);

	if (match == proofTypeNames.left.end())
		return (ostream << "Unknown");

	return (ostream << (*match).second);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::istream &operator>>(std::istream &istream, ProofType &proofType)
{
	std::string proofTypeName;
	istream >> proofTypeName;

	const auto match = proofTypeNames.right.find(proofTypeName);

	if (match == proofTypeNames.right.end())
		proofType = ProofType::Unknown;
	else
		proofType = (*match).second;

	return istream;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

}
}
}
