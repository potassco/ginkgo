#ifndef __FEEDBACK_LOOP__PRODUCTION__CLASP_CONSTRAINT_LOGGER_H
#define __FEEDBACK_LOOP__PRODUCTION__CLASP_CONSTRAINT_LOGGER_H

#include <clasp/clasp_facade.h>
#include <clasp/solver.h>

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

class ClaspConstraintLogger: public Clasp::EventHandler
{
	public:
		using LiteralName = std::pair<const char *, const char *>;
		using Symbol = std::pair<const char*, Clasp::Literal>;
		using SymbolTable = std::vector<Symbol>;

	public:
		ClaspConstraintLogger();

		void onEvent(const Clasp::Event &event) override;

		void log(const Clasp::Solver &solver, const Clasp::LitVec &literals,
			const Clasp::ConstraintInfo &constraintInfo);

	private:
		void readSymbolTable(const Clasp::OutputTable &outputTable);

		LiteralName literalName(Clasp::Literal literal) const;

		SymbolTable m_symbolTable;
		size_t m_numberOfSymbols;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

}
}
}

#endif
