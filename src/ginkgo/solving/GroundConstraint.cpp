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

void print(std::ostream &stream, const GroundConstraint &constraint)
{
	print(stream, constraint, ConstraintOutputFormat::Normal);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void printGeneralized(std::ostream &stream, const GroundConstraint &constraint)
{
	print(stream, constraint, ConstraintOutputFormat::Generalized, -constraint.timeRange().min);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void print(std::ostream &stream, const GroundConstraint &constraint,
	ConstraintOutputFormat outputFormat, int offset)
{
	stream << ":- ";

	const auto printTimeVariable =
		[&](const auto time)
		{
			stream << "T";

			if (time > 0)
				stream << "+" << time;
			else if (time < 0)
				stream << "-" << -time;
		};

	const auto printNormalizedLiteral =
		[&](const auto &literal)
		{
			const auto &clingoSymbol = literal.symbol()->clingoSymbol;

			stream << clingoSymbol.name();

			if (clingoSymbol.arguments().empty())
				return;

			stream << "(";

			for (auto i = clingoSymbol.arguments().begin(); i != clingoSymbol.arguments().end() - 1; i++)
			{
				const auto &argument = *i;

				if (i != clingoSymbol.arguments().begin())
					stream << ",";

				stream << argument;
			}

			if (clingoSymbol.arguments().size() > 1)
				stream << ",";

			const auto &timeArgument = clingoSymbol.arguments().back();
			const int time = timeArgument.number() + offset;

			if (outputFormat == ConstraintOutputFormat::Generalized)
				printTimeVariable(time);

			stream << ")";
		};

	for (auto i = constraint.literals().cbegin(); i != constraint.literals().cend(); i++)
	{
		const auto &literal = *i;

		if (i != constraint.literals().cbegin())
			stream << ", ";

		if (literal.sign() == Literal::Sign::Negative)
			stream << "not ";

		if (outputFormat == ConstraintOutputFormat::Normal)
			stream << literal.symbol()->clingoSymbol;
		else
			printNormalizedLiteral(literal);
	}

	if (outputFormat == ConstraintOutputFormat::Generalized)
	{
		const auto timeMin = static_cast<int>(constraint.timeRange().min) + offset;
		const auto timeMax = static_cast<int>(constraint.timeRange().max) + offset;

		for (auto time = timeMin; time <= timeMax; time++)
		{
			stream << ", time(";
			printTimeVariable(time);
			stream << ")";
		}
	}

	stream << ". %lbd = " << constraint.lbdAfterResolution();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::ostream &operator<<(std::ostream &stream, const GroundConstraint &constraint)
{
	print(stream, constraint);
	return stream;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool subsumes(const GroundConstraint &lhs, const GroundConstraint &rhs)
{
	return std::includes(lhs.literals().cbegin(), lhs.literals().cend(),
		rhs.literals().cbegin(), rhs.literals().cend());
}

////////////////////////////////////////////////////////////////////////////////////////////////////

}
