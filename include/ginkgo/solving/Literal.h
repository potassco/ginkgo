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
		Literal(Sign sign, const Symbol &symbol);

		Sign sign;
		const Symbol &symbol;
};

using Constraint = std::vector<Literal>;
using Constraints = std::vector<Constraint>;

////////////////////////////////////////////////////////////////////////////////////////////////////

}

#endif
