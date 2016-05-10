#include <ginkgo/solving/Satisfiability.h>

#include <iostream>
#include <boost/bimap.hpp>
#include <boost/assign.hpp>

namespace ginkgo
{

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Satisfiability
//
////////////////////////////////////////////////////////////////////////////////////////////////////

using SatisfiabilityNames = boost::bimap<Satisfiability, std::string>;
static SatisfiabilityNames satisfiabilityNames = boost::assign::list_of<SatisfiabilityNames::relation>
	(Satisfiability::Unknown, "Unknown")
	(Satisfiability::Satisfiable, "Satisfiable")
	(Satisfiability::Unsatisfiable, "Unsatisfiable");

////////////////////////////////////////////////////////////////////////////////////////////////////

std::ostream &operator<<(std::ostream &ostream, const Satisfiability &satisfiability)
{
	const auto match = satisfiabilityNames.left.find(satisfiability);

	if (match == satisfiabilityNames.left.end())
		return (ostream << "Unknown");

	return (ostream << (*match).second);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::istream &operator>>(std::istream &istream, Satisfiability &satisfiability)
{
	std::string satisfiabilityName;
	istream >> satisfiabilityName;

	const auto match = satisfiabilityNames.right.find(satisfiabilityName);

	if (match == satisfiabilityNames.right.end())
		satisfiability = Satisfiability::Unknown;
	else
		satisfiability = (*match).second;

	return istream;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

}
