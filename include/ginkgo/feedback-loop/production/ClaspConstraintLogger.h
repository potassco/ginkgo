#ifndef __FEEDBACK_LOOP__PRODUCTION__CLASP_CONSTRAINT_LOGGER_H
#define __FEEDBACK_LOOP__PRODUCTION__CLASP_CONSTRAINT_LOGGER_H

#include <clasp/clasp_facade.h>
#include <clasp/solver.h>

#include <ginkgo/solving/SymbolTable.h>

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
		enum class Sign
		{
			Negative = 0,
			Positive = 1
		};

		using LiteralName = std::pair<const char *, const char *>;

	public:
		explicit ClaspConstraintLogger(Clasp::EventHandler *childEventHandler);

		void onEvent(const Clasp::Event &event) override;

		void log(const Clasp::Solver &solver, const Clasp::LitVec &literals,
			const Clasp::ConstraintInfo &constraintInfo);

	private:
		void readSymbolTable(const Clasp::OutputTable &outputTable);

		LiteralName literalName(Clasp::Literal literal) const;

		Clasp::EventHandler *m_childEventHandler;

		SymbolTable m_symbolTable;
		size_t m_seenSymbols;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

}
}
}

#endif
