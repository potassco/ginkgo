#ifndef __SOLVING__CONSTRAINT_H
#define __SOLVING__CONSTRAINT_H

#include <set>

#include <ginkgo/solving/Literal.h>
#include <ginkgo/utils/Range.h>

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
		~Constraint() = default;

		Constraint(const Constraint &other) = default;
		Constraint &operator=(const Constraint &other) = default;

		Constraint(Constraint &&other) = default;
		Constraint &operator=(Constraint &&other) = default;

		Constraint withoutLiterals(size_t start, size_t number);

		size_t id() const;

		const Literals &literals() const;

		void setLBDOriginal(size_t lbdOriginal);
		size_t lbdOriginal() const;

		void setLBDAfterResolution(size_t lbdAfterResolution);
		size_t lbdAfterResolution() const;

		const Range<size_t> &timeRange() const;
		size_t degree() const;

		bool subsumes(const Constraint &other) const;

		void print(std::ostream &stream) const;
		void printGeneralized(std::ostream &stream) const;

	private:
		enum class OutputFormat
		{
			Normal,
			Generalized
		};

	private:
		void print(std::ostream &stream, OutputFormat outputFormat, int offset = 0) const;

		Range<size_t> computeTimeRange() const;

		size_t m_id;

		Literals m_literals;
		Range<size_t> m_timeRange;

		size_t m_lbdOriginal;
		size_t m_lbdAfterResolution;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

struct sortConstraints
{
	bool operator()(const Constraint &c1, const Constraint &c2)
	{
		const auto degree1 = c1.timeRange().max - c1.timeRange().min;
		const auto degree2 = c2.timeRange().max - c2.timeRange().min;

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
