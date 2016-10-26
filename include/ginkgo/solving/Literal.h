#ifndef __SOLVING__LITERAL_H
#define __SOLVING__LITERAL_H

#include <clingo.hh>

#include <clasp/solver.h>

#include <ginkgo/solving/SymbolTable.h>

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
		Literal(Clasp::Literal literal, const SymbolTable &symbolTable);

	public:
		Sign sign;
		Clasp::Literal claspLiteral;
		Clingo::Symbol symbol;
};

using Constraint = std::vector<Literal>;
using Constraints = std::vector<Constraint>;

////////////////////////////////////////////////////////////////////////////////////////////////////

}

#endif
