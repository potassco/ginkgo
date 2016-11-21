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

constexpr const auto StateWiseProofProgram =
R"(
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
		proofEncoding << FluentClosureProgram;
	else
		proofEncoding << StateGeneratorProgram;

	proofEncoding
		<< "#const degree=" << candidate.degree() << "." << std::endl
		<< "candidateConstraint(T) " << candidate << std::endl
		<< DegreeProgram
		<< ActionProgram
		<< StateWiseProofProgram << std::endl;

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
