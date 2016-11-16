#include <ginkgo/solving/GroundConstraint.h>

#include <boost/assert.hpp>

#include <ginkgo/solving/Constraint.h>

namespace ginkgo
{

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// GroundConstraint
//
////////////////////////////////////////////////////////////////////////////////////////////////////

GroundConstraint::GroundConstraint(size_t id, Literals &&literals, size_t lbdOriginal,
	size_t lbdAfterResolution)
:	m_id{id},
	m_literals{std::move(literals)},
	m_timeRange{computeTimeRange(m_literals)},
	m_lbdOriginal{lbdOriginal},
	m_lbdAfterResolution{lbdAfterResolution}
{
	BOOST_ASSERT_MSG(std::is_sorted(literals.cbegin(), literals.cend()),
		"Literal vector must be sorted");
}

////////////////////////////////////////////////////////////////////////////////////////////////////

GroundConstraint GroundConstraint::withoutLiterals(size_t start, size_t number) const
{
	BOOST_ASSERT(start + number <= m_literals.size());

	const auto numberOfLiterals = m_literals.size() - number;

	Literals literals;
	literals.reserve(numberOfLiterals);

	std::copy(m_literals.cbegin(), m_literals.cbegin() + start, std::back_inserter(literals));
	std::copy(m_literals.cbegin() + start + number, m_literals.cend(), std::back_inserter(literals));

	GroundConstraint result(m_id, std::move(literals), m_lbdOriginal, m_lbdAfterResolution);

	return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

size_t GroundConstraint::id() const
{
	return m_id;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

const Literals &GroundConstraint::literals() const
{
	return m_literals;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

const Range<size_t> &GroundConstraint::timeRange() const
{
	return m_timeRange;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

size_t GroundConstraint::degree() const
{
	return m_timeRange.max - m_timeRange.min;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

size_t GroundConstraint::lbdOriginal() const
{
	return m_lbdOriginal;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

size_t GroundConstraint::lbdAfterResolution() const
{
	return m_lbdAfterResolution;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::ostream &operator<<(std::ostream &stream, const GroundConstraint &constraint)
{
	stream << ":- ";

	for (auto i = constraint.literals().cbegin(); i != constraint.literals().cend(); i++)
	{
		const auto &literal = *i;

		if (i != constraint.literals().cbegin())
			stream << ", ";

		if (literal.sign == Literal::Sign::Negative)
			stream << "not ";

		stream << literal.symbol;
	}

	stream << ".";

	return stream;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

GroundConstraint parseGroundConstraint(const std::initializer_list<std::string> &constraint)
{
	Literals literals;

	std::for_each(constraint.begin(), constraint.end(),
		[&](const auto &argument)
		{
			auto sign = Literal::Sign::Positive;
			const auto *literalName = argument.c_str();

			if (argument.find("not ") != std::string::npos)
			{
				sign = Literal::Sign::Negative;
				literalName += 4;
			}

			const auto symbol = Clingo::parse_term(literalName);
			literals.emplace_back(Literal(sign, symbol));
		});

	std::sort(literals.begin(), literals.end());

	return GroundConstraint(0, std::move(literals), 0, 0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool subsumes(const GroundConstraint &lhs, const GroundConstraint &rhs)
{
	return subsumes(lhs.literals(), rhs.literals());
}

////////////////////////////////////////////////////////////////////////////////////////////////////

}
