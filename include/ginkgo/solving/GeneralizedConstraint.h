#ifndef __SOLVING__GENERALIZED_CONSTRAINT_H
#define __SOLVING__GENERALIZED_CONSTRAINT_H

#include <iosfwd>

#include <ginkgo/solving/Constraint.h>

namespace ginkgo
{
namespace deprecated
{

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// GeneralizedConstraint
//
////////////////////////////////////////////////////////////////////////////////////////////////////

class GeneralizedConstraint;

class GeneralizedConstraint
{
	public:
		GeneralizedConstraint(ConstraintPtr originalConstraint);
		GeneralizedConstraint(const GeneralizedConstraint &other);

		ConstraintPtr originalConstraint() const;

		size_t numberOfLiterals() const;

		size_t degree() const;
		int offset() const;

		bool subsumes(const Constraint &rhs) const;

		void print(std::ostream &ostream) const;

	private:
		// TODO: make const-correct
		ConstraintPtr m_originalConstraint;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

std::ostream &operator<<(std::ostream &ostream, const GeneralizedConstraint &generalizedConstraint);

////////////////////////////////////////////////////////////////////////////////////////////////////

}
}

#endif
