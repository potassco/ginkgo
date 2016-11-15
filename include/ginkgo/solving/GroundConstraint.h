#ifndef __SOLVING__GROUND_CONSTRAINT_H
#define __SOLVING__GROUND_CONSTRAINT_H

#include <set>

#include <ginkgo/solving/Literal.h>
#include <ginkgo/utils/Range.h>

namespace ginkgo
{

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// GroundConstraint
//
////////////////////////////////////////////////////////////////////////////////////////////////////

class GroundConstraint
{
	public:
		GroundConstraint(size_t id, Literals &&literals);
		~GroundConstraint() = default;

		GroundConstraint(const GroundConstraint &other) = default;
		GroundConstraint &operator=(const GroundConstraint &other) = default;

		GroundConstraint(GroundConstraint &&other) = default;
		GroundConstraint &operator=(GroundConstraint &&other) = default;

		GroundConstraint withoutLiterals(size_t start, size_t number);

		size_t id() const;

		const Literals &literals() const;

		void setLBDOriginal(size_t lbdOriginal);
		size_t lbdOriginal() const;

		void setLBDAfterResolution(size_t lbdAfterResolution);
		size_t lbdAfterResolution() const;

		const Range<size_t> &timeRange() const;
		size_t degree() const;

		bool subsumes(const GroundConstraint &other) const;

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

struct sortGroundConstraints
{
	bool operator()(const GroundConstraint &c1, const GroundConstraint &c2)
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

using GroundConstraints = std::vector<GroundConstraint>;
using GroundConstraintBuffer = std::set<GroundConstraint, sortGroundConstraints>;

////////////////////////////////////////////////////////////////////////////////////////////////////

std::ostream &operator<<(std::ostream &stream, const GroundConstraint &constraint);

////////////////////////////////////////////////////////////////////////////////////////////////////

}

#endif
