#ifndef __SOLVING__GENERALIZED_CONSTRAINT_H
#define __SOLVING__GENERALIZED_CONSTRAINT_H

#include <set>

#include <ginkgo/solving/GroundConstraint.h>
#include <ginkgo/solving/Literal.h>
#include <ginkgo/utils/Range.h>

namespace ginkgo
{

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// GeneralizedConstraint
//
////////////////////////////////////////////////////////////////////////////////////////////////////

class GeneralizedConstraint
{
	public:
		GeneralizedConstraint(GroundConstraint &groundConstraint);

		GeneralizedConstraint withoutLiterals(size_t start, size_t number) const;

		const Literals &literals() const;
		size_t offset() const;
		size_t degree() const;

	private:
		GeneralizedConstraint() = default;

		Literals m_literals;
		int m_offset;
		size_t m_degree;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

using GeneralizedConstraints = std::vector<GeneralizedConstraint>;

////////////////////////////////////////////////////////////////////////////////////////////////////

std::ostream &operator<<(std::ostream &stream, const GeneralizedConstraint &constraint);

bool subsumes(const GeneralizedConstraint &lhs, const GeneralizedConstraint &rhs);
bool subsumes(const GeneralizedConstraint &lhs, const GroundConstraint &rhs);

////////////////////////////////////////////////////////////////////////////////////////////////////

}

#endif
