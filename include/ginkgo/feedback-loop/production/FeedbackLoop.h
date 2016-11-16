#ifndef __FEEDBACK_LOOP__PRODUCTION__FEEDBACK_LOOP_H
#define __FEEDBACK_LOOP__PRODUCTION__FEEDBACK_LOOP_H

#include <condition_variable>
#include <mutex>
#include <sstream>

#include <ginkgo/feedback-loop/production/ClaspConstraintLogger.h>
#include <ginkgo/feedback-loop/production/Environment.h>
#include <ginkgo/feedback-loop/production/Events.h>
#include <ginkgo/feedback-loop/production/ProofResult.h>
#include <ginkgo/solving/GroundConstraint.h>
#include <ginkgo/solving/GeneralizedConstraint.h>

namespace ginkgo
{
namespace feedbackLoop
{
namespace production
{

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// FeedbackLoop
//
////////////////////////////////////////////////////////////////////////////////////////////////////

class FeedbackLoop
{
	public:
		// Computes the actual answer sets of an encoding
		static const std::string MetaEncoding;
		// Generates initial states
		static const std::string StateGeneratorEncoding;
		// Generates initial states with the fluent closure via forward chaining
		static const std::string FluentClosureEncoding;
		// Check whether the hypothesis holds
		static const std::string StateWiseProofEncoding;
		static const std::string InductiveProofBaseEncoding;
		static const std::string InductiveProofStepEncoding;

	public:
		FeedbackLoop(std::unique_ptr<Environment> environment, std::unique_ptr<Configuration<Plain>> configuration);

		void run();

	private:
		void mergePrograms();
		void prepareExtraction();
		GeneralizedConstraint minimizeConstraint(const GeneralizedConstraint &provenConstraint, size_t linearIncrement);
		ProofResult testCandidateStateWise(const GeneralizedConstraint &candidate, EventHypothesisTested::Purpose purpose);
		ProofResult testCandidateInductively(const GeneralizedConstraint &candidate, EventHypothesisTested::Purpose purpose);

		std::unique_ptr<Environment> m_environment;
		std::unique_ptr<Configuration<Plain>> m_configuration;

		std::unique_ptr<ClaspConstraintLogger> m_claspConstraintLogger;

		GroundConstraintBuffer m_extractedConstraints;
		GeneralizedConstraints m_provenConstraints;

		Events m_events;

		std::stringstream m_program;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

}
}
}

#endif
