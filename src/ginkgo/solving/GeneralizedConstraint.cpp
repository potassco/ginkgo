#include <ginkgo/solving/GeneralizedConstraint.h>

namespace ginkgo
{
namespace deprecated
{

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// GeneralizedConstraint
//
////////////////////////////////////////////////////////////////////////////////////////////////////

GeneralizedConstraint::GeneralizedConstraint(ConstraintPtr originalConstraint)
:	m_originalConstraint{originalConstraint}
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////

GeneralizedConstraint::GeneralizedConstraint(const GeneralizedConstraint &other)
:	m_originalConstraint{other.originalConstraint()}
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////

ConstraintPtr GeneralizedConstraint::originalConstraint() const
{
	return m_originalConstraint;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

size_t GeneralizedConstraint::numberOfLiterals() const
{
	return m_originalConstraint->numberOfLiterals();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

size_t GeneralizedConstraint::degree() const
{
	const auto timeRange = m_originalConstraint->timeRange();
	const auto &timeMin = std::get<0>(timeRange);
	const auto &timeMax = std::get<1>(timeRange);

	return timeMax - timeMin;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

int GeneralizedConstraint::offset() const
{
	const auto &timeMin = std::get<0>(m_originalConstraint->timeRange());

	return -timeMin;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool GeneralizedConstraint::subsumes(const Constraint &rhs) const
{
	// LHS: generalized constraint (normalized to 0, but holds even if shifted in time)
	const auto lhsDegree = degree();
	const auto lhsOffset = offset();

	// RHS: other constraint
	const auto rhsTimeRange = rhs.timeRange();
	const auto &rhsTimeMin = std::get<0>(rhsTimeRange);
	const auto &rhsTimeMax = std::get<1>(rhsTimeRange);

	for (size_t t0 = rhsTimeMin; t0 + lhsDegree <= rhsTimeMax; t0++)
	{
		// Shift the generalized constraint and test it against the other constraint
		if (m_originalConstraint->subsumes(rhs, lhsOffset + t0))
			return true;
	}

	return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void GeneralizedConstraint::print(std::ostream &ostream) const
{
	m_originalConstraint->print(ostream, Literal::OutputFormat::Generalized, offset());
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::ostream &operator<<(std::ostream &ostream, const GeneralizedConstraint &generalizedConstraint)
{
	generalizedConstraint.print(ostream);

	return ostream;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

}
}
