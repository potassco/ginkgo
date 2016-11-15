#ifndef __FEEDBACK_LOOP__PRODUCTION__CLASP_CONSTRAINT_LOGGER_H
#define __FEEDBACK_LOOP__PRODUCTION__CLASP_CONSTRAINT_LOGGER_H

#include <condition_variable>
#include <mutex>
#include <sstream>

#include <clasp/clasp_facade.h>
#include <clasp/solver.h>
#include <clingo.hh>

#include <ginkgo/feedback-loop/production/Configuration.h>
#include <ginkgo/solving/Literal.h>
#include <ginkgo/solving/GroundConstraint.h>

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
		ClaspConstraintLogger(std::stringstream &program, GroundConstraintBuffer &constraintBuffer, const Configuration<Plain> &configuration);

		void fill(size_t constraintBufferSize);
		void terminate();

	private:
		enum class State
		{
			Filling,
			Full,
			Terminated
		};

	private:
		void onEvent(const Clasp::Event &event) override;
		void log(const Clasp::Solver &solver, const Clasp::LitVec &literals,
			const Clasp::ConstraintInfo &constraintInfo);
		void readSymbolTable(const Clasp::OutputTable &outputTable);

		State m_state;

		Clingo::Control m_clingoControl;
		std::unique_ptr<Clingo::SolveAsync> m_clingoSolveAsync;

		Clasp::EventHandler *m_childEventHandler;

		GroundConstraintBuffer &m_constraintBuffer;
		size_t m_currentConstraintID;

		Symbols m_symbols;
		size_t m_seenSymbols;

		size_t m_constraintBufferSize;
		std::condition_variable m_constraintBufferCondition;
		std::mutex m_constraintBufferMutex;

		const Configuration<Plain> &m_configuration;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

}
}
}

#endif
