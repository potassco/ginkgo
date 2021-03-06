#ifndef __FEEDBACK_LOOP__PRODUCTION__FEEDBACK_LOOP_H
#define __FEEDBACK_LOOP__PRODUCTION__FEEDBACK_LOOP_H

#include <mutex>
#include <condition_variable>

#include <ginkgo/feedback-loop/production/Environment.h>
#include <ginkgo/feedback-loop/production/ProofResult.h>
#include <ginkgo/feedback-loop/production/Events.h>

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
		static const std::string InductionProofBaseEncoding;
		static const std::string InductionProofStepEncoding;

	public:
		FeedbackLoop(std::unique_ptr<Environment> environment, std::unique_ptr<Configuration<Plain>> configuration);
		~FeedbackLoop();

		void run();

	private:
		void mergeEncodings();
		void generateFeedback(size_t constraintsToExtract, bool startOver = true);
		GeneralizedConstraint minimizeConstraint(const GeneralizedConstraint &provenGeneralizedConstraint, size_t linearIncrement);
		ProofResult testHypothesisStateWise(const GeneralizedConstraint &generalizedHypothesis, EventHypothesisTested::Purpose purpose);
		ProofResult testHypothesisInduction(const GeneralizedConstraint &generalizedHypothesis, EventHypothesisTested::Purpose purpose);

		std::unique_ptr<Environment> m_environment;
		std::unique_ptr<Configuration<Plain>> m_configuration;

		AsyncProcess m_gringo;
		AsyncProcess m_clasp;
		AsyncProcess m_xclasp;

		std::condition_variable m_pauseCondition;
		std::mutex m_pauseConditionMutex;

		Constraints m_feedback;

		Events m_events;

		std::stringstream m_program;
		Constraints m_learnedConstraints;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

}
}
}

#endif
