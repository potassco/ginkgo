#include <ginkgo/solving/Literal.h>

namespace ginkgo
{

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Literal
//
////////////////////////////////////////////////////////////////////////////////////////////////////

Literal::Literal(Clasp::Literal literal, const SymbolTable &symbolTable)
{
	const auto &claspSymbol = symbolTable.at(literal.var());

	symbol = Clingo::parse_term(claspSymbol.first);
	sign = (claspSymbol.second == literal ? Sign::Positive : Sign::Negative);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

}
