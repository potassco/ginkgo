#include <ginkgo/feedback-loop/production/ClaspConstraintLogger.h>

#include <iostream>

namespace ginkgo
{
namespace feedbackLoop
{
namespace production
{

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// ClaspConstraintLogger
//
////////////////////////////////////////////////////////////////////////////////////////////////////

ClaspConstraintLogger::ClaspConstraintLogger()
:	m_seenSymbols{0}
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void ClaspConstraintLogger::onEvent(const Clasp::Event &event)
{
	const auto conflictEvent = Clasp::event_cast<Clasp::NewConflictEvent>(event);

	if (conflictEvent)
		log(*conflictEvent->solver, *conflictEvent->learnt, conflictEvent->info);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void ClaspConstraintLogger::log(const Clasp::Solver &solver, const Clasp::LitVec &literals,
	const Clasp::ConstraintInfo &constraintInfo)
{
	if (solver.id() != 0)
		throw std::runtime_error("warning: multi-threading currently unsupported");

	const auto &outputTable = solver.sharedContext()->output;

	readSymbolTable(outputTable);

	const uint32_t MAX_LBD = 127;
	uint32_t lbd = constraintInfo.lbd();

	const auto allowedVariables = Clasp::VarInfo::Input | Clasp::VarInfo::Output;

	Clasp::LitVec output;

	if (lbd > MAX_LBD || !solver.resolveToFlagged(literals, allowedVariables, output, lbd) || lbd > MAX_LBD)
	{
		std::cout << "warning: skipped conflict" << std::endl;
		return;
	}

	std::cout << "[constraint] :- ";

	std::for_each(output.begin(), output.end(),
		[&](const auto &literal)
		{
			const auto outputLiteral = ~literal;
			const auto name = this->literalName(outputLiteral);
			std::cout << name.first << name.second << ", ";
		});

	std::cout << std::endl;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void ClaspConstraintLogger::readSymbolTable(const Clasp::OutputTable &outputTable)
{
	if (m_seenSymbols == outputTable.size())
		return;

	std::for_each(outputTable.pred_begin(), outputTable.pred_end(),
		[&](const auto &predicate)
		{
			const auto variable = predicate.cond.var();

			if (m_symbolTable.size() <= variable)
				m_symbolTable.resize(variable + 1, Symbol("<?>", Clasp::lit_false()));

			if (m_symbolTable[variable].first == 0 || (!predicate.cond.sign() && m_symbolTable[variable].second.sign()))
				m_symbolTable[variable] = Symbol(predicate.name.c_str(), predicate.cond);
		});

	m_seenSymbols = outputTable.size();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

ClaspConstraintLogger::LiteralName ClaspConstraintLogger::literalName(Clasp::Literal literal) const
{
	const auto &symbol = m_symbolTable.at(literal.var());

	return {symbol.second == literal ? "" : "not ", symbol.first};
}

////////////////////////////////////////////////////////////////////////////////////////////////////

}
}
}
