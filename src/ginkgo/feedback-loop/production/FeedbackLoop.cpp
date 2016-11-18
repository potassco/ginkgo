#include <ginkgo/feedback-loop/production/FeedbackLoop.h>

#include <iostream>
#include <limits>
#include <thread>

#include <clingo.hh>
#include <clasp/clasp_facade.h>

#include <json/json.h>

#include <ginkgo/utils/TextFile.h>
#include <ginkgo/solving/Satisfiability.h>

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

const std::string FeedbackLoop::MetaEncoding =
	R"(
	% Horizon
	time(0..horizon).

	% Establish initial state
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

	% Verify that goal is met
	:- goal(F, true), not holds(F, horizon).
	:- goal(F, false), holds(F, horizon).
	)";

////////////////////////////////////////////////////////////////////////////////////////////////////

const std::string FeedbackLoop::StateGeneratorEncoding =
	R"(
	% Generate any possible initial state
	{holds(F, 0)} :- fluent(F).
	)";

////////////////////////////////////////////////////////////////////////////////////////////////////

const std::string FeedbackLoop::FluentClosureEncoding =
	R"(
	% Iteratively build the fluent closure using forward chaining
	{holds(F, 0)} :- fluentClosure(F).
	fluentClosure(F) :- init(F).
	fluentClosure(F1) :- action(A), fluentClosure(F2) : demands(A, F2, true), adds(A, F1).
	)";

////////////////////////////////////////////////////////////////////////////////////////////////////

const std::string FeedbackLoop::StateWiseProofEncoding =
	R"(
	% Degree of the candidate
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

	% Eliminate all states complying with the constraint
	:- not candidateConstraint(0).
	)";

////////////////////////////////////////////////////////////////////////////////////////////////////

const std::string FeedbackLoop::InductiveProofBaseEncoding =
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
	":- not candidateConstraint(0).
	)";

////////////////////////////////////////////////////////////////////////////////////////////////////

const std::string FeedbackLoop::InductiveProofStepEncoding =
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

FeedbackLoop::FeedbackLoop(std::unique_ptr<Environment> environment, std::unique_ptr<Configuration<Plain>> configuration)
:	m_environment(std::move(environment)),
	m_configuration(std::move(configuration))
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void FeedbackLoop::run()
{
	setlocale(LC_NUMERIC, "C");

	mergePrograms();

	m_events.startTimer();

	if (m_environment->logLevel() == LogLevel::Debug)
		std::cout << "[Info ] Starting initial constraint extraction" << std::endl;

	prepareExtraction();

	m_claspConstraintLogger->fill(m_configuration->constraintsToExtract);

	if (m_environment->logLevel() == LogLevel::Debug)
		std::cout << "[Info ] Finished initial constraint extraction" << std::endl;

	if (m_extractedConstraints.empty())
		// No more constraints, exiting
		return;

	// TODO: reimplement statistics

	while (!m_extractedConstraints.empty())
	{
		auto extractedConstraint = std::move(*m_extractedConstraints.begin());
		m_extractedConstraints.erase(m_extractedConstraints.begin());
		m_claspConstraintLogger->fill(m_configuration->constraintsToExtract);

		const auto subsumed = std::find_if(m_provenConstraints.cbegin(), m_provenConstraints.cend(),
			[&](const auto &provenConstraint)
			{
				return subsumes(provenConstraint, extractedConstraint);
			}) != m_provenConstraints.cend();

		if (subsumed)
		{
			std::cout << "[Info ] \033[1;33mskipped conflict (subsumed by already proven generalized constraint)\033[0m" << std::endl;
			continue;
		}

		GeneralizedConstraint candidate(extractedConstraint);

		if (m_environment->logLevel() == LogLevel::Debug)
			std::cout << "[Info ] Testing candidate (degree: " << candidate.degree()
				<< ", #literals: " << candidate.literals().size() << ")" << std::endl;

		auto proofResult = ProofResult::Unknown;

		switch (m_configuration->proofMethod)
		{
			case ProofMethod::StateWise:
				proofResult = testCandidateStateWise(candidate, EventCandidateTested::Purpose::Prove);
				break;
			case ProofMethod::Inductive:
				proofResult = testCandidateInductively(candidate, EventCandidateTested::Purpose::Prove);
				break;
			default:
				std::cerr << "[Error] Unknown proof method" << std::endl;
				break;
		}

		if (proofResult == ProofResult::Unknown)
		{
			std::cerr << "[Error] Invalid proof result" << std::endl;
			continue;
		}

		if (proofResult == ProofResult::Unproven)
		{
			if (m_environment->logLevel() == LogLevel::Debug)
				std::cout << "[Info ] \033[1;31mCandidate unproven\033[0m" << std::endl;

			continue;
		}

		if (proofResult == ProofResult::GroundingTimeout || proofResult == ProofResult::SolvingTimeout)
		{
			if (m_environment->logLevel() == LogLevel::Debug)
				std::cout << "[Info ] \033[1;33mTimeout proving candidate\033[0m" << std::endl;

			continue;
		}

		BOOST_ASSERT(proofResult == ProofResult::Proven);

		if (m_configuration->minimizationStrategy == MinimizationStrategy::SimpleMinimization)
			candidate = minimizeConstraint(candidate, 0);
		else if (m_configuration->minimizationStrategy == MinimizationStrategy::LinearMinimization)
			candidate = minimizeConstraint(candidate, 1);

		m_provenConstraints.push_back(candidate);

		std::cout << "[Info ] \033[1;32mCandidate proven\033[0m" << " (" << m_provenConstraints.size();

		if (m_configuration->constraintsToProve > 0)
			std::cout << "/" << m_configuration->constraintsToProve;

		std::cout << ")" << std::endl;

		auto &directConstraintsStream = m_environment->directConstraintsStream();
		auto &generalizedConstraintsStream = m_environment->generalizedConstraintsStream();

		directConstraintsStream << extractedConstraint << std::endl;
		generalizedConstraintsStream << candidate << std::endl;
		m_program << candidate << std::endl;

		// Stop if we have proven enough constraints
		if (m_configuration->constraintsToProve > 0
		    && m_provenConstraints.size() >= m_configuration->constraintsToProve)
		{
			break;
		}
	}

	// Gracefully terminate clasp constraint logger to avoid deadlocks
	m_claspConstraintLogger->terminate();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void FeedbackLoop::mergePrograms()
{
	if (!m_program.str().empty())
	{
		m_program.str(std::string());
		m_program.clear();
	}

	for (const auto &inputFileName : {m_configuration->instance, m_configuration->domain})
	{
		TextFile inputFile(inputFileName);
		m_program << inputFile.read().rdbuf() << std::endl;
	}

	std::for_each(m_provenConstraints.cbegin(), m_provenConstraints.cend(),
		[&](const auto &provenConstraint)
		{
			m_program << provenConstraint << std::endl;
		});
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void FeedbackLoop::prepareExtraction()
{
	m_program.clear();
	m_program.seekg(0, std::ios::beg);

	std::stringstream metaEncoding;
	metaEncoding
		<< "#const horizon=" << m_configuration->horizon << "." << std::endl
		<< MetaEncoding << std::endl
		<< m_program.rdbuf() << std::endl;

	metaEncoding.clear();
	metaEncoding.seekg(0, std::ios::beg);

	m_claspConstraintLogger = std::make_unique<ClaspConstraintLogger>(metaEncoding, m_extractedConstraints, *m_configuration);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

GeneralizedConstraint FeedbackLoop::minimizeConstraint(const GeneralizedConstraint &provenConstraint, size_t linearIncrement)
{
	//const auto literalsBefore = provenGeneralizedConstraint.numberOfLiterals();
	//const auto degreeBefore = provenGeneralizedConstraint.degree();
	size_t requiredTests = 0;

	// If nothing works, keep the original constraint
	auto result = provenConstraint;

	if (m_environment->logLevel() == LogLevel::Debug)
		std::cout << "[Info ] Trying to minimize " << provenConstraint << std::endl;

	// Start with windows of size 1
	size_t windowSize = 1;

	for (size_t i = 0; i < result.literals().size();)
	{
		if (m_environment->logLevel() == LogLevel::Debug)
			std::cout << "[Info ] Trying to eliminate " << windowSize << " literals starting at " << i << std::endl;

		auto candidate = result.withoutLiterals(i, windowSize);

		// Skip candidates that have become empty due to removing literals
		if (candidate.literals().empty())
		{
			if (m_environment->logLevel() == LogLevel::Debug)
				std::cout << "[Info ] Skipped empty candidate property" << std::endl;

			i++;
			continue;
		}

		ProofResult proofResult = ProofResult::Unknown;

		switch (m_configuration->proofMethod)
		{
			case ProofMethod::StateWise:
				proofResult = testCandidateStateWise(candidate, EventCandidateTested::Purpose::Minimize);
				break;
			case ProofMethod::Inductive:
				proofResult = testCandidateInductively(candidate, EventCandidateTested::Purpose::Minimize);
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

	if (m_environment->logLevel() == LogLevel::Debug)
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

ProofResult FeedbackLoop::testCandidateStateWise(const GeneralizedConstraint &candidate, EventCandidateTested::Purpose purpose)
{
	m_program.clear();
	m_program.seekg(0, std::ios::beg);

	std::stringstream proofEncoding;
	proofEncoding << m_program.rdbuf();

	if (m_configuration->fluentClosureUsage == FluentClosureUsage::UseFluentClosure)
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
	const auto finished = solveAsync.wait(m_configuration->candidateTestingTimeout.count());

	if (!finished)
		return ProofResult::SolvingTimeout;

	if (satisfiable == Satisfiability::Unsatisfiable)
		return ProofResult::Proven;
	else
		return ProofResult::Unproven;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

ProofResult FeedbackLoop::testCandidateInductively(const GeneralizedConstraint &candidate, EventCandidateTested::Purpose purpose)
{
	m_program.clear();
	m_program.seekg(0, std::ios::beg);

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Induction Base
	////////////////////////////////////////////////////////////////////////////////////////////////
	{
		std::stringstream proofEncoding;
		proofEncoding
			<< m_program.rdbuf()
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
		const auto finished = solveAsync.wait(m_configuration->candidateTestingTimeout.count());

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

	m_program.clear();
	m_program.seekg(0, std::ios::beg);

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Induction Step
	////////////////////////////////////////////////////////////////////////////////////////////////
	{
		std::stringstream proofEncoding;
		proofEncoding
			<< m_program.rdbuf();

		if (m_configuration->fluentClosureUsage == FluentClosureUsage::UseFluentClosure)
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
		const auto finished = solveAsync.wait(m_configuration->candidateTestingTimeout.count());

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
