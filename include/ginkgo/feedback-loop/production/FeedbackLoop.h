#ifndef __FEEDBACK_LOOP__PRODUCTION__FEEDBACK_LOOP_H
#define __FEEDBACK_LOOP__PRODUCTION__FEEDBACK_LOOP_H

#include <condition_variable>
#include <mutex>
#include <sstream>

#include <ginkgo/feedback-loop/production/ClaspConstraintLogger.h>
#include <ginkgo/feedback-loop/production/Environment.h>
#include <ginkgo/feedback-loop/production/Events.h>
#include <ginkgo/feedback-loop/production/ProofResult.h>
#include <ginkgo/solving/Constraint.h>
#include <ginkgo/solving/__deprecated__GeneralizedConstraint.h>

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
		void mergeEncodings();
		void prepareExtraction();
		deprecated::GeneralizedConstraint minimizeConstraint(const deprecated::GeneralizedConstraint &provenGeneralizedConstraint, size_t linearIncrement);
		ProofResult testHypothesisStateWise(const Constraint &candidate, EventHypothesisTested::Purpose purpose);
		ProofResult testHypothesisInductively(const Constraint &candidate, EventHypothesisTested::Purpose purpose);

		std::unique_ptr<Environment> m_environment;
		std::unique_ptr<Configuration<Plain>> m_configuration;

		std::unique_ptr<ClaspConstraintLogger> m_claspConstraintLogger;

		ConstraintBuffer m_extractedConstraints;
		Constraints m_provenConstraints;

		Events m_events;

		std::stringstream m_program;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

}
}
}

#endif
