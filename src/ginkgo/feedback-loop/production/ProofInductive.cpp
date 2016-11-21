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

% Establish initial state
holds(Variable, Value, 0) :- initialState(Variable, Value).

% Perform actions
1 {occurs(Action, T) : action(Action)} 1 :- time(T), T > 0.

% Check preconditions
:- occurs(Action, T), precondition(Action, Variable, Value), not holds(Variable, Value, T - 1), time(T), time(T - 1).

% Apply effects
caused(Variable, Value, T) :- occurs(Action, T), postcondition(Action, _, Variable, Value).
modified(Variable, T) :- caused(Variable, Value, T).

holds(Variable, Value, T) :- caused(Variable, Value, T), time(T).
holds(Variable, Value, T) :- holds(Variable, Value, T - 1), not modified(Variable, T), time(T), time(T - 1).

% Check that variables have unique values
:- variable(Variable), not 1 {holds(Variable, Value, T) : contains(Variable, Value)} 1, time(T).

% Check mutexes
:- mutexGroup(MutexGroup), not {holds(Variable, Value, T) : contains(MutexGroup, Variable, Value)} 1, time(T).

% Eliminate all states complying with the constraint at t = 0
:- not candidateConstraint(0).
)";

////////////////////////////////////////////////////////////////////////////////////////////////////

constexpr const auto InductiveProofStepEncoding =
R"(
% Degree of the candidate (+ 1)
time(0..degree).

% Perform actions
1 {occurs(Action, T) : action(Action)} 1 :- time(T), T > 0.

% Check preconditions
:- occurs(Action, T), precondition(Action, Variable, Value), not holds(Variable, Value, T - 1), time(T), time(T - 1).

% Apply effects
caused(Variable, Value, T) :- occurs(Action, T), postcondition(Action, _, Variable, Value).
modified(Variable, T) :- caused(Variable, Value, T).

holds(Variable, Value, T) :- caused(Variable, Value, T), time(T).
holds(Variable, Value, T) :- holds(Variable, Value, T - 1), not modified(Variable, T), time(T), time(T - 1).

% Check that variables have unique values
:- variable(Variable), not 1 {holds(Variable, Value, T) : contains(Variable, Value)} 1, time(T).

% Check mutexes
:- mutexGroup(MutexGroup), not {holds(Variable, Value, T) : contains(MutexGroup, Variable, Value)} 1, time(T).

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
