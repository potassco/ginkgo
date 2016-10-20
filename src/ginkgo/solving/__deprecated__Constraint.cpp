#include <ginkgo/solving/__deprecated__Constraint.h>

#include <iostream>
#include <sstream>
#include <algorithm>
#include <limits>
#include <boost/assert.hpp>

#include <ginkgo/utils/Utils.h>

namespace ginkgo
{
namespace deprecated
{

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Constraint
//
////////////////////////////////////////////////////////////////////////////////////////////////////

Constraint::Constraint(size_t id, const std::string &string, SymbolTable &symbolTable)
:	m_id(id),
	m_lbd(0)
{
	size_t position = 0;

	while (position < string.size())
	{
		if (string[position] == '.')
			break;

		if (!isAlphanumeric(string[position]))
		{
			position++;
			continue;
		}

		Literal literal(string, position, symbolTable);

		if (std::find(m_literals.cbegin(), m_literals.cend(), literal) == m_literals.cend())
			m_literals.push_back(literal);
	}

	const std::string lbdPattern = "lbd=";
	size_t lbdPosition = string.find(lbdPattern, position);

	if (lbdPosition != std::string::npos)
	{
		std::stringstream stream(string);
		stream.seekg(lbdPosition + lbdPattern.size());
		stream >> m_lbd;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Constraint::Constraint(size_t id, Literals literals, size_t lbd)
:	m_id(id),
	m_literals(literals),
	m_lbd(lbd)
{
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

size_t Constraint::numberOfLiterals() const
{
	return m_literals.size();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

size_t Constraint::lbd() const
{
	return m_lbd;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool Constraint::subsumes(const Constraint &rhs) const
{
	for (const auto literal : m_literals)
		if (std::find(rhs.literals().begin(), rhs.literals().end(), literal) == rhs.literals().end())
			return false;

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool Constraint::subsumes(const Constraint &rhs, int offset) const
{
	if (offset == 0)
		return subsumes(rhs);

	for (const auto lhsLiteral : m_literals)
	{
		if (std::find_if(rhs.literals().begin(), rhs.literals().end(),
			[&](const auto &rhsLiteral)
			{
				return lhsLiteral.equalsShifted(rhsLiteral, offset);
			})
			== rhs.literals().end())
		{
			return false;
		}
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool Constraint::containsIdentifier(const std::string &identifier) const
{
	auto result = std::find_if(m_literals.cbegin(), m_literals.cend(), [&identifier](const auto &literal)
	{
		return *literal.name() == identifier;
	});

	return result != m_literals.cend();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::ostream &operator<<(std::ostream &ostream, const Constraint &constraint)
{
	constraint.print(ostream);

	return ostream;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::tuple<size_t, size_t> Constraint::timeRange() const
{
	// Currently, normalization only works for plasp-formatted encodings
	std::for_each(m_literals.cbegin(), m_literals.cend(), [](const auto &literal)
	{
		BOOST_ASSERT_MSG(Literal::SupportedTimeIdentifiers.find(*literal.name()) != Literal::SupportedTimeIdentifiers.end(),
			"Identifier unsupported");
	});

	size_t timeMin = std::numeric_limits<size_t>::max();
	size_t timeMax = std::numeric_limits<size_t>::min();

	for (const auto &literal : m_literals)
	{
		const auto time = literal.timeArgument();

		// Actions require at least one preceding time step in order to check preconditions
		if (*literal.name() == "apply" || *literal.name() == "del")
			timeMin = std::min(timeMin, time - 1);
		else
			timeMin = std::min(timeMin, time);

		timeMax = std::max(timeMax, time);
	}

	return std::make_tuple(timeMin, timeMax);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

ConstraintPtr Constraint::withoutLiterals(size_t index, size_t length) const
{
	BOOST_ASSERT(index < m_literals.size());

	ConstraintPtr result(new Constraint);
	result->m_id = m_id;
	result->m_lbd = 0;

	// If all constraints shall be removed, return the empty copy
	if (length >= m_literals.size())
		return result;

	result->m_literals.reserve(m_literals.size() - length);

	for (size_t i = 0; i < m_literals.size(); i++)
		if (i < index || i >= index + length)
			result->m_literals.push_back(m_literals[i]);

	return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Constraint::removeLiterals(const std::string &literalName)
{
	m_literals.erase(std::remove_if(m_literals.begin(), m_literals.end(),
		[&](auto &literal)
		{
			return *literal.name() == literalName;
		}), m_literals.end());
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Constraint::print(std::ostream &ostream) const
{
	ostream << ":- ";

	for (size_t i = 0; i < m_literals.size(); i++)
	{
		if (i != 0)
			ostream << ", ";

		ostream << m_literals[i];
	}

	ostream << ".";
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Constraint::print(std::ostream &ostream, Literal::OutputFormat outputFormat, int offset) const
{
	ostream << ":- ";

	if (outputFormat == Literal::OutputFormat::Generalized)
	{
		const auto timeRange = this->timeRange();
		const auto &timeMin = static_cast<int>(std::get<0>(timeRange)) + offset;
		const auto &timeMax = static_cast<int>(std::get<1>(timeRange)) + offset;

		for (auto time = timeMin; time <= timeMax; time++)
		{
			ostream << "time(T";

			if (time < 0)
				ostream << "-" << time;
			else if (time > 0)
				ostream << "+" << time;

			ostream << "), ";
		}
	}

	for (size_t i = 0; i < m_literals.size(); i++)
	{
		if (i != 0)
			ostream << ", ";

		m_literals[i].print(ostream, outputFormat, offset);
	}

	ostream << ".";
}

////////////////////////////////////////////////////////////////////////////////////////////////////

}
}
