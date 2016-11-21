#include <ginkgo/feedback-loop/production/ProofStateWise.h>

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
// ProofStateWise
//
////////////////////////////////////////////////////////////////////////////////////////////////////

constexpr const auto StateWiseProofEncoding =
R"(
% Degree of the candidate
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

% Eliminate all states complying with the constraint
:- not candidateConstraint(0).
)";

////////////////////////////////////////////////////////////////////////////////////////////////////

ProofResult testCandidateStateWise(const GeneralizedConstraint &candidate, std::istream &program,
	const Configuration<Plain> &configuration)
{
	program.clear();
	program.seekg(0, std::ios::beg);

	std::stringstream proofEncoding;
	proofEncoding << program.rdbuf();

	if (configuration.fluentClosureUsage == FluentClosureUsage::UseFluentClosure)
		proofEncoding << FluentClosureEncoding;
	else
		proofEncoding << StateGeneratorEncoding;

	proofEncoding
		<< "#const degree=" << candidate.degree() << "." << std::endl
		<< "candidateConstraint(T) ";

	proofEncoding
		<< candidate << std::endl
		<< StateWiseProofEncoding << std::endl;

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

	if (satisfiable == Satisfiability::Unsatisfiable)
		return ProofResult::Proven;
	else
		return ProofResult::Unproven;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

}
}
}