#ifndef __FEEDBACK_LOOP__PRODUCTION__CLASP_CONSTRAINT_LOGGER_H
#define __FEEDBACK_LOOP__PRODUCTION__CLASP_CONSTRAINT_LOGGER_H

#include <clasp/clasp_facade.h>
#include <clasp/solver.h>

#include <ginkgo/solving/Literal.h>
#include <ginkgo/solving/Constraint.h>

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
		ClaspConstraintLogger(Clasp::EventHandler *childEventHandler, Constraints &constraints);

		void onEvent(const Clasp::Event &event) override;

		void log(const Clasp::Solver &solver, const Clasp::LitVec &literals,
			const Clasp::ConstraintInfo &constraintInfo);

	private:
		void readSymbolTable(const Clasp::OutputTable &outputTable);

		Clasp::EventHandler *m_childEventHandler;

		Constraints &m_constraints;

		Symbols m_symbols;
		size_t m_seenSymbols;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

}
}
}

#endif
