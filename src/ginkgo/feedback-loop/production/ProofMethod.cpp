#include <ginkgo/feedback-loop/production/ProofMethod.h>

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
// ProofMethod
//
////////////////////////////////////////////////////////////////////////////////////////////////////

using ProofMethodNames = boost::bimap<ProofMethod, std::string>;
static ProofMethodNames proofMethodNames = boost::assign::list_of<ProofMethodNames::relation>
	(ProofMethod::StateWise, "StateWise")
	(ProofMethod::Inductive, "Inductive");

////////////////////////////////////////////////////////////////////////////////////////////////////

std::ostream &operator<<(std::ostream &ostream, const ProofMethod &proofMethod)
{
	const auto match = proofMethodNames.left.find(proofMethod);

	if (match == proofMethodNames.left.end())
		return (ostream << "Unknown");

	return (ostream << (*match).second);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::istream &operator>>(std::istream &istream, ProofMethod &proofMethod)
{
	std::string proofMethodName;
	istream >> proofMethodName;

	const auto match = proofMethodNames.right.find(proofMethodName);

	if (match == proofMethodNames.right.end())
		proofMethod = ProofMethod::Unknown;
	else
		proofMethod = (*match).second;

	return istream;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

}
}
}
