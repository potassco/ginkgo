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

		const Literals &literals() const;

		void setLBDOriginal(size_t lbdOriginal);
		size_t lbdOriginal() const;

		void setLBDAfterResolution(size_t lbdAfterResolution);
		size_t lbdAfterResolution() const;

		std::tuple<size_t, size_t> timeRange() const;

		void print(std::ostream &stream) const;
		void printNormalized(std::ostream &stream, int offset) const;
		void printGeneralized(std::ostream &stream, int offset) const;

	private:
		enum class OutputFormat
		{
			Normal,
			Normalized,
			Generalized
		};

	private:
		void print(std::ostream &stream, OutputFormat outputFormat, int offset = 0) const;

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
		// TODO: don’t copy
		const auto timeRange1 = c1.timeRange();
		const auto timeRange2 = c2.timeRange();

		const auto degree1 = std::get<1>(timeRange1) - std::get<0>(timeRange1);
		const auto degree2 = std::get<1>(timeRange2) - std::get<0>(timeRange2);

		if (degree1 != degree2)
			return degree1 < degree2;

		if (c1.literals().size() != c2.literals().size())
			return c1.literals().size() < c2.literals().size();

		return c1.id() < c2.id();
	}
};

////////////////////////////////////////////////////////////////////////////////////////////////////

using Constraints = std::vector<Constraint>;
using ConstraintBuffer = std::set<Constraint, sortConstraints>;

////////////////////////////////////////////////////////////////////////////////////////////////////

std::ostream &operator<<(std::ostream &stream, const Constraint &constraint);

////////////////////////////////////////////////////////////////////////////////////////////////////

}

#endif
