#ifndef __SOLVING__LITERAL_H
#define __SOLVING__LITERAL_H

#include <clingo.hh>

#include <clasp/solver.h>

#include <ginkgo/solving/Symbol.h>

namespace ginkgo
{

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Literal
//
////////////////////////////////////////////////////////////////////////////////////////////////////

class Literal
{
	public:
		enum class Sign
		{
			Negative = 0,
			Positive = 1
		};

	public:
		Literal(Sign sign, const Symbol *symbol);

		Sign sign() const;
		const Symbol *symbol() const;

		friend bool operator<(const Literal &lhs, const Literal &rhs)
		{
			if (lhs.m_symbol->clingoSymbol != rhs.m_symbol->clingoSymbol)
				return lhs.m_symbol->clingoSymbol < rhs.m_symbol->clingoSymbol;

			return lhs.m_sign < rhs.m_sign;
		}

	private:
		Sign m_sign;
		const Symbol *m_symbol;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

using Literals = std::vector<Literal>;

////////////////////////////////////////////////////////////////////////////////////////////////////

}

#endif
