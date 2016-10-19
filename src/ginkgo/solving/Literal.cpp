#include <ginkgo/solving/Literal.h>

#include <iostream>
#include <sstream>
#include <algorithm>
#include <boost/assert.hpp>

#include <ginkgo/utils/Utils.h>

namespace ginkgo
{
namespace deprecated
{

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Literal
//
////////////////////////////////////////////////////////////////////////////////////////////////////

// TODO: verify if terminal can be safely removed
const std::set<std::string> Literal::SupportedTimeIdentifiers =
	{"del", "holds", "apply", "terminal"};

////////////////////////////////////////////////////////////////////////////////////////////////////

Literal::Literal(const std::string &string, size_t &startPosition, SymbolTable &symbolTable)
:	m_name{nullptr},
	m_hasTimeArgument{false}
{
	// Check sign
	m_sign = string.substr(startPosition, 4) != "not ";

	if (!m_sign)
		startPosition += 4;

	auto position = startPosition;

	// Extract name
	while (position < string.size())
	{
		if (!isAlphanumeric(string[position]))
			break;

		position++;
	}

	BOOST_ASSERT_MSG(position < string.size(), "Could not parse literal.");

	const auto name = string.substr(startPosition, position - startPosition);
	m_name = symbolTable.identifier(name);

	// Check for arguments
	if (string[position] != '(')
		startPosition = position;
	else
	{
		const auto leftBracketPosition = position;
		const auto rightBracketPosition = findMatchingRightParenthesis(string, position);

		// Extract arguments
		position = leftBracketPosition + 1;

		while (position < rightBracketPosition)
		{
			if (!isAlphanumeric(string[position]))
			{
				position++;
				continue;
			}

			m_arguments.emplace_back(Literal(string, position, symbolTable));
		}

		startPosition = rightBracketPosition + 1;
	}

	if (!m_arguments.empty()
		&& SupportedTimeIdentifiers.find(*m_name) != SupportedTimeIdentifiers.end())
	{
		try
		{
			m_timeArgument = std::stoul(*m_arguments.back().name());
			m_hasTimeArgument = true;
		}
		catch (const std::exception &e)
		{
			std::cout << "[Warn ] Reading time argument failed: " << e.what() << std::endl;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Literal::Literal(const Literal &copy)
:	m_sign(copy.m_sign),
	m_name(copy.m_name),
	m_arguments(copy.m_arguments),
	m_hasTimeArgument(copy.m_hasTimeArgument),
	m_timeArgument(copy.m_timeArgument)
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool Literal::sign() const
{
	return m_sign;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

const std::string *Literal::name() const
{
	BOOST_ASSERT(m_name);

	return m_name;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

const Literals &Literal::arguments() const
{
	return m_arguments;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool Literal::hasTimeArgument() const
{
	return m_hasTimeArgument;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

size_t Literal::timeArgument() const
{
	BOOST_ASSERT_MSG(m_hasTimeArgument, "Literal has no valid time argument");

	return m_timeArgument;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool Literal::equalsShifted(const Literal &rhs, int offset) const
{
	if (m_sign != rhs.sign())
		return false;

	if (m_arguments.size() != rhs.arguments().size())
		return false;

	if (hasTimeArgument() != rhs.hasTimeArgument())
		return false;

	// Compare all but the last argument (time argument)
	for (size_t i = 0; i < m_arguments.size() - 1; i++)
	{
		if (m_arguments[i] != rhs.arguments()[i])
			return false;
	}

	// Compare time argument with offset
	if (timeArgument() + offset != rhs.timeArgument())
		return false;

	if (m_name != rhs.name())
		return false;

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Literal Literal::normalized(int offset, SymbolTable &symbolTable) const
{
	BOOST_ASSERT(!m_arguments.empty());

	Literal result;
	result.m_name = m_name;
	result.m_sign = m_sign;
	result.m_arguments = m_arguments;

	if (offset == 0)
		return result;

	result.m_arguments.back().m_name = symbolTable.identifier(std::to_string(timeArgument() + offset));

	return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::ostream &operator<<(std::ostream &ostream, const Literal &literal)
{
	literal.print(ostream);

	return ostream;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool operator==(const Literal &a, const Literal &b)
{
	if (a.sign() != b.sign())
		return false;

	if (a.name() != b.name())
		return false;

	const auto &aArguments = a.arguments();
	const auto &bArguments = b.arguments();

	if (aArguments.size() != bArguments.size())
		return false;

	for (size_t i = 0; i < aArguments.size(); i++)
		if (aArguments[i] != bArguments[i])
			return false;

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool operator!=(const Literal &a, const Literal &b)
{
	return !(a == b);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Literal::print(std::ostream &ostream) const
{
	if (m_sign == false)
		ostream << "not ";

	ostream << *m_name;

	if (!m_arguments.empty())
	{
		ostream << "(";

		for (size_t i = 0; i < m_arguments.size(); i++)
		{
			if (i != 0)
				ostream << ", ";

			ostream << m_arguments[i];
		}

		ostream << ")";
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Literal::print(std::ostream &ostream, OutputFormat outputFormat, int offset) const
{
	// Currently, normalization only works for plasp-formatted encodings
	BOOST_ASSERT_MSG(SupportedTimeIdentifiers.find(*m_name) != SupportedTimeIdentifiers.end(),
		"Identifier unsupported");

	if (m_sign == false)
		ostream << "not ";

	ostream << *m_name;

	if (!m_arguments.empty())
	{
		ostream << "(";

		// Don't touch all but the time argument (last argument)
		for (size_t i = 0; i < m_arguments.size() - 1; i++)
		{
			if (i != 0)
				ostream << ", ";

			ostream << m_arguments[i];
		}

		if (m_arguments.size() > 1)
			ostream << ", ";

		const auto time = static_cast<int>(timeArgument()) + offset;

		// Normalize time argument
		switch (outputFormat)
		{
			case OutputFormat::Normal:
				ostream << time;
				break;

			case OutputFormat::Generalized:
			{
				ostream << "T";

				if (time < 0)
					ostream << "-" << time;
				else if (time > 0)
					ostream << "+" << time;

				break;
			}
		}

		ostream << ")";
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////

}
}
