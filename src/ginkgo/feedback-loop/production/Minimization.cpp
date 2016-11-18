#include <ginkgo/feedback-loop/production/Minimization.h>

#include <sstream>

#include <ginkgo/feedback-loop/production/PartialPrograms.h>
#include <ginkgo/feedback-loop/production/ProofInductive.h>
#include <ginkgo/feedback-loop/production/ProofStateWise.h>
#include <ginkgo/solving/Satisfiability.h>

namespace ginkgo
{
namespace feedbackLoop
{
namespace production
{

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Minimization
//
////////////////////////////////////////////////////////////////////////////////////////////////////

// TODO: refactor without using environment
GeneralizedConstraint minimizeConstraint(const GeneralizedConstraint &provenConstraint,
	size_t linearIncrement, std::istream &program, const Configuration<Plain> &configuration,
	const Environment &environment)
{
	//const auto literalsBefore = provenGeneralizedConstraint.numberOfLiterals();
	//const auto degreeBefore = provenGeneralizedConstraint.degree();
	size_t requiredTests = 0;

	// If nothing works, keep the original constraint
	auto result = provenConstraint;

	if (environment.logLevel() == LogLevel::Debug)
		std::cout << "[Info ] Trying to minimize " << provenConstraint << std::endl;

	// Start with windows of size 1
	size_t windowSize = 1;

	for (size_t i = 0; i < result.literals().size();)
	{
		if (environment.logLevel() == LogLevel::Debug)
			std::cout << "[Info ] Trying to eliminate " << windowSize << " literals starting at " << i << std::endl;

		auto candidate = result.withoutLiterals(i, windowSize);

		// Skip candidates that have become empty due to removing literals
		if (candidate.literals().empty())
		{
			if (environment.logLevel() == LogLevel::Debug)
				std::cout << "[Info ] Skipped empty candidate property" << std::endl;

			i++;
			continue;
		}

		ProofResult proofResult = ProofResult::Unknown;

		switch (configuration.proofMethod)
		{
			case ProofMethod::StateWise:
				proofResult = testCandidateStateWise(candidate, program, configuration);
				break;
			case ProofMethod::Inductive:
				proofResult = testCandidateInductively(candidate, program, configuration);
				break;
			default:
				std::cerr << "[Error] Unknown proof method" << std::endl;
				break;
		}

		requiredTests++;

		if (proofResult == ProofResult::Unknown)
		{
			std::cerr << "[Error] Invalid proof result" << std::endl;
			continue;
		}

		if (proofResult == ProofResult::Unproven
			|| proofResult == ProofResult::GroundingTimeout
			|| proofResult == ProofResult::SolvingTimeout)
		{
			// Try again with smallest window size
			if (windowSize > 1)
			{
				windowSize = 1;
				continue;
			}

			i++;
			continue;
		}

		// If proven, then just keep the minimized candidate and try to minimize further
		result = candidate;

		// Also increase the window size by 1
		windowSize += linearIncrement;
	}

	if (environment.logLevel() == LogLevel::Debug)
	{
		std::cout << "[Info ] \033[1;34mEliminated "
			<< (provenConstraint.literals().size() - result.literals().size())
			<< "/" << provenConstraint.literals().size()
			<< " literals through minimization\033[0m" << std::endl;
	}

	// TODO: reimplement statistics

	return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

}
}
}
