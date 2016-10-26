#include <ginkgo/feedback-loop/production/ClaspConstraintLogger.h>

#include <iostream>

#include <ginkgo/solving/Literal.h>

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

ClaspConstraintLogger::ClaspConstraintLogger(Clasp::EventHandler *childEventHandler, Constraints &constraints)
:	m_childEventHandler{childEventHandler},
	m_constraints(constraints),
	m_seenSymbols{0}
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void ClaspConstraintLogger::onEvent(const Clasp::Event &event)
{
	m_childEventHandler->onEvent(event);

	const auto conflictEvent = Clasp::event_cast<Clasp::NewConflictEvent>(event);

	if (conflictEvent)
		log(*conflictEvent->solver, *conflictEvent->learnt, conflictEvent->info);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void ClaspConstraintLogger::log(const Clasp::Solver &solver, const Clasp::LitVec &claspLiterals,
	const Clasp::ConstraintInfo &constraintInfo)
{
	if (solver.id() != 0)
		throw std::runtime_error("warning: multi-threading currently unsupported");

	const auto &outputTable = solver.sharedContext()->output;

	readSymbolTable(outputTable);

	const uint32_t maxLBD = 127;
	uint32_t lbd = constraintInfo.lbd();

	const auto allowedVariables = Clasp::VarInfo::Input | Clasp::VarInfo::Output;

	Clasp::LitVec output;

	if (lbd > maxLBD)
	{
		std::cout << "\033[1;31mwarning: skipped conflict (LBD too high: " << lbd << ", maximum allowed: " << maxLBD << ")\033[0m" << std::endl;
		return;
	}

	if (!solver.resolveToFlagged(claspLiterals, allowedVariables, output, lbd))
	{
		std::cout << "\033[1;33mwarning: skipped conflict (cannot be resolved to selected variables)\033[0m" << std::endl;
		return;
	}

	//std::cout << ":- ";

	Constraint constraint;
	constraint.reserve(output.size());

	for (auto i = output.begin(); i != output.end(); i++)
	{
		//if (i != output.begin())
		//	std::cout << ", ";

		const auto literal = ~*i;

		const auto &symbol = m_symbolTable.at(literal.var());
		const auto sign = (symbol.claspLiteral == literal ? Literal::Sign::Positive : Literal::Sign::Negative);

		constraint.emplace_back(Literal(sign, symbol));

		//std::cout << (sign == Literal::Sign::Negative ? "not " : "") << symbol.name;
	}

	m_constraints.emplace_back(constraint);

	//std::cout << ".  %lbd = " << lbd << std::endl;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void ClaspConstraintLogger::readSymbolTable(const Clasp::OutputTable &outputTable)
{
	if (m_seenSymbols == outputTable.size())
		return;

	const char *noName = "<?>";

	std::for_each(outputTable.pred_begin(), outputTable.pred_end(),
		[&](const auto &predicate)
		{
			const auto variable = predicate.cond.var();

			if (m_symbolTable.size() <= variable)
				m_symbolTable.resize(variable + 1, {noName, Clingo::Symbol(), Clasp::lit_false()});

			if (m_symbolTable[variable].name == noName || (!predicate.cond.sign() && m_symbolTable[variable].claspLiteral.sign()))
				m_symbolTable[variable] = {predicate.name.c_str(), Clingo::parse_term(predicate.name.c_str()), predicate.cond};
		});

	m_seenSymbols = outputTable.size();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

}
}
}
