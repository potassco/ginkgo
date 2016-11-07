#include <ginkgo/solving/Constraint.h>

#include <boost/assert.hpp>

namespace ginkgo
{

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Constraint
//
////////////////////////////////////////////////////////////////////////////////////////////////////

Constraint::Constraint(size_t id, Literals &&literals)
:	m_id{id},
	m_literals{std::move(literals)},
	m_timeRange{computeTimeRange()},
	m_lbdOriginal{0},
	m_lbdAfterResolution{0}
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Constraint Constraint::withoutLiterals(size_t start, size_t number)
{
	BOOST_ASSERT(start + number <= m_literals.size());

	const auto numberOfLiterals = m_literals.size() - number;

	Literals literals;
	literals.reserve(numberOfLiterals);

	std::copy(m_literals.begin(), m_literals.begin() + start, std::back_inserter(literals));
	std::copy(m_literals.begin() + start + number, m_literals.end(), std::back_inserter(literals));

	Constraint result(m_id, std::move(literals));
	result.m_lbdOriginal = m_lbdOriginal;
	result.m_lbdAfterResolution = m_lbdAfterResolution;

	return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

size_t Constraint::id() const
{
	return m_id;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

const Literals &Constraint::literals() const
{
	return m_literals;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

const Range<size_t> &Constraint::timeRange() const
{
	return m_timeRange;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

size_t Constraint::degree() const
{
	return m_timeRange.max - m_timeRange.min;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Constraint::setLBDOriginal(size_t lbdOriginal)
{
	m_lbdOriginal = lbdOriginal;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

size_t Constraint::lbdOriginal() const
{
	return m_lbdOriginal;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Constraint::setLBDAfterResolution(size_t lbdAfterResolution)
{
	m_lbdAfterResolution = lbdAfterResolution;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

size_t Constraint::lbdAfterResolution() const
{
	return m_lbdAfterResolution;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Range<size_t> Constraint::computeTimeRange() const
{
	// Currently, normalization only works for plasp-formatted encodings
	/*std::for_each(m_literals.cbegin(), m_literals.cend(), [](const auto &literal)
	{
		BOOST_ASSERT_MSG(Literal::SupportedTimeIdentifiers.find(*literal.name()) != Literal::SupportedTimeIdentifiers.end(),
			"Identifier unsupported");
	});*/

	Range<size_t> timeRange{std::numeric_limits<size_t>::max(), std::numeric_limits<size_t>::min()};

	for (const auto &literal : m_literals)
	{
		BOOST_ASSERT(!literal.symbol()->clingoSymbol.arguments().empty());

		const auto &timeArgument = literal.symbol()->clingoSymbol.arguments().back();
		const size_t time = timeArgument.number();

		// Actions require at least one preceding time step in order to check preconditions
		if (std::strcmp(literal.symbol()->clingoSymbol.name(), "apply") == 0 || std::strcmp(literal.symbol()->clingoSymbol.name(), "del") == 0)
			timeRange.min = std::min(timeRange.min, time - 1);
		else
			timeRange.min = std::min(timeRange.min, time);

		timeRange.max = std::max(timeRange.max, time);
	}

	return timeRange;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Constraint::print(std::ostream &stream) const
{
	print(stream, OutputFormat::Normal);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Constraint::printGeneralized(std::ostream &stream) const
{
	print(stream, OutputFormat::Generalized, -m_timeRange.min);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Constraint::print(std::ostream &stream, Constraint::OutputFormat outputFormat, int offset) const
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

			if (outputFormat == OutputFormat::Generalized)
				printTimeVariable(time);

			stream << ")";
		};

	for (auto i = m_literals.cbegin(); i != m_literals.cend(); i++)
	{
		const auto &literal = *i;

		if (i != m_literals.cbegin())
			stream << ", ";

		if (literal.sign() == Literal::Sign::Negative)
			stream << "not ";

		if (outputFormat == OutputFormat::Normal)
			stream << literal.symbol()->clingoSymbol;
		else
			printNormalizedLiteral(literal);
	}

	if (outputFormat == OutputFormat::Generalized)
	{
		// TODO: don’t copy
		const auto timeRange = this->timeRange();
		const auto timeMin = static_cast<int>(timeRange.min) + offset;
		const auto timeMax = static_cast<int>(timeRange.max) + offset;

		for (auto time = timeMin; time <= timeMax; time++)
		{
			stream << ", time(";
			printTimeVariable(time);
			stream << ")";
		}
	}

	stream << ". %lbd = " << m_lbdAfterResolution;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::ostream &operator<<(std::ostream &stream, const Constraint &constraint)
{
	constraint.print(stream);

	return stream;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

}
