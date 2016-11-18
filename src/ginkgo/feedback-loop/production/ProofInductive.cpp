#include <ginkgo/feedback-loop/production/ProofInductive.h>

#include <sstream>

#include <ginkgo/feedback-loop/production/PartialPrograms.h>
#include <ginkgo/solving/Satisfiability.h>

namespace ginkgo
{
namespace feedbackLoop
{
namespace production
{

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// ProofInductive
//
////////////////////////////////////////////////////////////////////////////////////////////////////

constexpr const auto InductiveProofBaseEncoding =
R"(
% Degree of the candidate
time(0..degree).

% Establish the initial state
holds(F, 0) :- init(F).

% Perform actions
1 {apply(A, T) : action(A)} 1 :- time(T), T > 0.

% Check preconditions
:- apply(A, T), demands(A, F, true), not holds(F, T - 1), time(T), time(T - 1).
:- apply(A, T), demands(A, F, false), holds(F, T - 1), time(T), time(T - 1).

% Apply effects
holds(F, T) :- apply(A, T), adds(A, F), action(A), time(T).
del(F, T) :- apply(A, T), deletes(A, F), action(A), time(T).
holds(F, T) :- holds(F, T - 1), not del(F, T), time(T), time(T - 1).

% Eliminate all states complying with the constraint at t = 0
:- not candidateConstraint(0).
)";

////////////////////////////////////////////////////////////////////////////////////////////////////

constexpr const auto InductiveProofStepEncoding =
R"(
% Degree of the candidate (+ 1)
time(0..degree).

% Perform actions
1 {apply(A, T) : action(A)} 1 :- time(T), T > 0.

% Check preconditions
:- apply(A, T), demands(A, F, true), not holds(F, T - 1), time(T), time(T - 1).
:- apply(A, T), demands(A, F, false), holds(F, T - 1), time(T), time(T - 1).

% Apply effects
holds(F, T) :- apply(A, T), adds(A, F), action(A), time(T).
del(F, T) :- apply(A, T), deletes(A, F), action(A), time(T).
holds(F, T) :- holds(F, T - 1), not del(F, T), time(T), time(T - 1).

% Eliminate all states not complying with the constraint at t = 0
:- candidateConstraint(0).

% Eliminate all states complying with the constraint at t = 1
:- not candidateConstraint(1).
)";

////////////////////////////////////////////////////////////////////////////////////////////////////

ProofResult testCandidateInductively(const GeneralizedConstraint &candidate, std::istream &program,
	const Configuration<Plain> &configuration)
{
	program.clear();
	program.seekg(0, std::ios::beg);

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Induction Base
	////////////////////////////////////////////////////////////////////////////////////////////////
	{
		std::stringstream proofEncoding;
		proofEncoding
			<< program.rdbuf()
			<< "#const degree=" << candidate.degree() << "." << std::endl
			<< "candidateConstraint(T) ";

		proofEncoding
			<< candidate << std::endl
			<< InductiveProofBaseEncoding << std::endl;

		// TODO: add warning/error message handler
		// TODO: record statistics, including grounding time
		Clingo::Control clingoControl{{"--stats=2"}};
		clingoControl.add("base", {}, proofEncoding.str().c_str());
		clingoControl.ground({{"base", {}}});

		auto satisfiable = Satisfiability::Unsatisfiable;

		const auto handleModel =
			[&satisfiable](const auto &model)
			{
				satisfiable = Satisfiability::Satisfiable;
				return true;
			};

		auto solveAsync = clingoControl.solve_async(handleModel);
		const auto finished = solveAsync.wait(configuration.candidateTestingTimeout.count());

		if (!finished)
			return ProofResult::SolvingTimeout;

		ProofResult proofResult = ProofResult::Unknown;

		if (satisfiable == Satisfiability::Unsatisfiable)
			proofResult = ProofResult::Proven;
		else
			proofResult = ProofResult::Unproven;

		if (proofResult == ProofResult::Unproven
			|| proofResult == ProofResult::GroundingTimeout
			|| proofResult == ProofResult::SolvingTimeout)
			return proofResult;
	}

	program.clear();
	program.seekg(0, std::ios::beg);

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Induction Step
	////////////////////////////////////////////////////////////////////////////////////////////////
	{
		std::stringstream proofEncoding;
		proofEncoding
			<< program.rdbuf();

		if (configuration.fluentClosureUsage == FluentClosureUsage::UseFluentClosure)
			proofEncoding << FluentClosureEncoding;
		else
			proofEncoding << StateGeneratorEncoding;

		proofEncoding
			<< "#const degree=" << (candidate.degree() + 1) << "." << std::endl
			<< "candidateConstraint(T) ";

		proofEncoding
			<< candidate << std::endl
			<< InductiveProofStepEncoding << std::endl;

		proofEncoding.clear();
		proofEncoding.seekg(0, std::ios::beg);

		// TODO: add warning/error message handler
		// TODO: record statistics, including grounding time
		Clingo::Control clingoControl{{"--stats=2"}};
		clingoControl.add("base", {}, proofEncoding.str().c_str());
		clingoControl.ground({{"base", {}}});

		auto satisfiable = Satisfiability::Unsatisfiable;

		const auto handleModel =
			[&satisfiable](const auto &model)
			{
				satisfiable = Satisfiability::Satisfiable;
				return true;
			};

		auto solveAsync = clingoControl.solve_async(handleModel);
		const auto finished = solveAsync.wait(configuration.candidateTestingTimeout.count());

		if (!finished)
			return ProofResult::SolvingTimeout;

		ProofResult proofResult = ProofResult::Unknown;

		if (satisfiable == Satisfiability::Unsatisfiable)
			proofResult = ProofResult::Proven;
		else
			proofResult = ProofResult::Unproven;

		return proofResult;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////

}
}
}
