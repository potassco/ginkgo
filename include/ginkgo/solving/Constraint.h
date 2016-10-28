#ifndef __SOLVING__CONSTRAINT_H
#define __SOLVING__CONSTRAINT_H

#include <set>

#include <ginkgo/solving/Literal.h>

namespace ginkgo
{

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Constraint
//
////////////////////////////////////////////////////////////////////////////////////////////////////

class Constraint
{
	public:
		Constraint(size_t id, Literals &&literals);

		size_t id() const;

		Literals &literals();
		const Literals &literals() const;

		void setLBDOriginal(size_t lbdOriginal);
		size_t lbdOriginal() const;

		void setLBDAfterResolution(size_t lbdAfterResolution);
		size_t lbdAfterResolution() const;

	private:
		size_t m_id;

		Literals m_literals;

		size_t m_lbdOriginal;
		size_t m_lbdAfterResolution;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

struct sortConstraints
{
	bool operator()(const Constraint &c1, const Constraint &c2)
	{
		if (c1.literals().size() == c2.literals().size())
			return c1.id() < c2.id();

		return c1.literals().size() < c2.literals().size();
	}
};

////////////////////////////////////////////////////////////////////////////////////////////////////

using Constraints = std::set<Constraint, sortConstraints>;

////////////////////////////////////////////////////////////////////////////////////////////////////

std::ostream &operator<<(std::ostream &stream, const Constraint &constraint);

////////////////////////////////////////////////////////////////////////////////////////////////////

}

#endif
