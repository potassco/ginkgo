#ifndef __SOLVING__LITERAL_H
#define __SOLVING__LITERAL_H

#include <iosfwd>
#include <string>
#include <memory>
#include <vector>
#include <set>

#include <ginkgo/solving/SymbolTable.h>

namespace ginkgo
{
namespace deprecated
{

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Literal
//
////////////////////////////////////////////////////////////////////////////////////////////////////

class Literal;
// TODO: make const
using Literals = std::vector<Literal>;

class Literal
{
	public:
		static const std::set<std::string> SupportedTimeIdentifiers;

		enum class OutputFormat
		{
			Normal,
			Generalized
		};

	public:
		Literal(const std::string &string, size_t &startPosition, SymbolTable &symbolTable);
		Literal(const Literal &copy);

		bool sign() const;
		const std::string *name() const;
		const Literals &arguments() const;
		bool hasTimeArgument() const;
		size_t timeArgument() const;

		bool equalsShifted(const Literal &rhs, int offset) const;

		Literal normalized(int offset, SymbolTable &symbolTable) const;

		void print(std::ostream &ostream) const;
		void print(std::ostream &ostream, OutputFormat outputFormat, int offset = 0) const;

	private:
		Literal() = default;

		bool m_sign;
		const std::string *m_name;
		Literals m_arguments;

		bool m_hasTimeArgument;
		size_t m_timeArgument;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

std::ostream &operator<<(std::ostream &ostream, const Literal &literal);
bool operator==(const Literal &a, const Literal &b);
bool operator!=(const Literal &a, const Literal &b);

////////////////////////////////////////////////////////////////////////////////////////////////////

}
}

#endif
