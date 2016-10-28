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
	m_lbdOriginal{0},
	m_lbdAfterResolution{0}
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////

size_t Constraint::id() const
{
	return m_id;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Literals &Constraint::literals()
{
	return m_literals;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

const Literals &Constraint::literals() const
{
	return m_literals;
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

std::tuple<size_t, size_t> Constraint::timeRange() const
{
	// Currently, normalization only works for plasp-formatted encodings
	/*std::for_each(m_literals.cbegin(), m_literals.cend(), [](const auto &literal)
	{
		BOOST_ASSERT_MSG(Literal::SupportedTimeIdentifiers.find(*literal.name()) != Literal::SupportedTimeIdentifiers.end(),
			"Identifier unsupported");
	});*/

	size_t timeMin = std::numeric_limits<size_t>::max();
	size_t timeMax = std::numeric_limits<size_t>::min();

	for (const auto &literal : m_literals)
	{
		BOOST_ASSERT(!literal.symbol().clingoSymbol.arguments().empty());

		const auto &timeArgument = literal.symbol().clingoSymbol.arguments().back();
		const size_t time = timeArgument.number();

		// Actions require at least one preceding time step in order to check preconditions
		if (std::strcmp(literal.symbol().clingoSymbol.name(), "apply") == 0 || std::strcmp(literal.symbol().clingoSymbol.name(), "del") == 0)
			timeMin = std::min(timeMin, time - 1);
		else
			timeMin = std::min(timeMin, time);

		timeMax = std::max(timeMax, time);
	}

	return std::make_tuple(timeMin, timeMax);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Constraint::print(std::ostream &stream) const
{
	print(stream, OutputFormat::Normal);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Constraint::printNormalized(std::ostream &stream, int offset) const
{
	print(stream, OutputFormat::Normalized, offset);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Constraint::printGeneralized(std::ostream &stream, int offset) const
{
	print(stream, OutputFormat::Generalized, offset);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Constraint::print(std::ostream &stream, Constraint::OutputFormat outputFormat, int offset) const
{
	stream << ":- ";

	const auto printNormalizedLiteral =
		[&](const auto &literal)
		{
			const auto &clingoSymbol = literal.symbol().clingoSymbol;

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

			if (outputFormat == OutputFormat::Normalized)
				stream << time;
			else if (outputFormat == OutputFormat::Generalized)
			{
				stream << "T";

				if (time > 0)
					stream << "+" << time;
				else if (time < 0)
					stream << "-" << -time;
			}

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
			stream << literal.symbol().clingoSymbol;
		else
			printNormalizedLiteral(literal);
	}

	stream << ".  %lbd = " << m_lbdAfterResolution;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::ostream &operator<<(std::ostream &stream, const Constraint &constraint)
{
	constraint.print(stream);

	return stream;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

}
