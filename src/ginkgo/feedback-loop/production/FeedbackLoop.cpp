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
	// Horizon
	"time(0..horizon).\n"
	// Establish initial state
	"holds(F, 0) :- init(F).\n"
	// Perform actions
	"1 {apply(A, T) : action(A)} 1 :- time(T), T > 0.\n"
	// Check preconditions
	":- apply(A, T), demands(A, F, true), not holds(F, T - 1), time(T), time(T - 1).\n"
	":- apply(A, T), demands(A, F, false), holds(F, T - 1), time(T), time(T - 1).\n"
	// Apply effects
	"holds(F, T) :- apply(A, T), adds(A, F), action(A), time(T).\n"
	"del(F, T) :- apply(A, T), deletes(A, F), action(A), time(T).\n"
	"holds(F, T) :- holds(F, T - 1), not del(F, T), time(T), time(T - 1).\n"
	// Verify that goal is met
	":- goal(F, true), not holds(F, horizon).\n"
	":- goal(F, false), holds(F, horizon).\n";

////////////////////////////////////////////////////////////////////////////////////////////////////

const std::string FeedbackLoop::StateGeneratorEncoding =
	// Generate any possible initial state
	"{holds(F, 0)} :- fluent(F).\n";

////////////////////////////////////////////////////////////////////////////////////////////////////

const std::string FeedbackLoop::FluentClosureEncoding =
	// Iteratively build the fluent closure using forward chaining
	"{holds(F, 0)} :- fluentClosure(F).\n"
	"fluentClosure(F) :- init(F).\n"
	"fluentClosure(F1) :- action(A), fluentClosure(F2) : demands(A, F2, true), adds(A, F1).\n";

////////////////////////////////////////////////////////////////////////////////////////////////////

const std::string FeedbackLoop::StateWiseProofEncoding =
	// Degree of the hypothesis
	"time(0..degree).\n"
	// Perform actions
	"1 {apply(A, T) : action(A)} 1 :- time(T), T > 0.\n"
	// Check preconditions
	":- apply(A, T), demands(A, F, true), not holds(F, T - 1), time(T), time(T - 1).\n"
	":- apply(A, T), demands(A, F, false), holds(F, T - 1), time(T), time(T - 1).\n"
	// Apply effects
	"holds(F, T) :- apply(A, T), adds(A, F), action(A), time(T).\n"
	"del(F, T) :- apply(A, T), deletes(A, F), action(A), time(T).\n"
	"holds(F, T) :- holds(F, T - 1), not del(F, T), time(T), time(T - 1).\n"
	// Eliminate all states complying with the constraint
	":- not hypothesisConstraint(0).\n";

////////////////////////////////////////////////////////////////////////////////////////////////////

const std::string FeedbackLoop::InductiveProofBaseEncoding =
	// Degree of the hypothesis
	"time(0..degree).\n"
	// Establish the initial state
	"holds(F, 0) :- init(F).\n"
	// Perform actions
	"1 {apply(A, T) : action(A)} 1 :- time(T), T > 0.\n"
	// Check preconditions
	":- apply(A, T), demands(A, F, true), not holds(F, T - 1), time(T), time(T - 1).\n"
	":- apply(A, T), demands(A, F, false), holds(F, T - 1), time(T), time(T - 1).\n"
	// Apply effects
	"holds(F, T) :- apply(A, T), adds(A, F), action(A), time(T).\n"
	"del(F, T) :- apply(A, T), deletes(A, F), action(A), time(T).\n"
	"holds(F, T) :- holds(F, T - 1), not del(F, T), time(T), time(T - 1).\n"
	// Eliminate all states complying with the constraint at t = 0
	":- not hypothesisConstraint(0).\n";

////////////////////////////////////////////////////////////////////////////////////////////////////

const std::string FeedbackLoop::InductiveProofStepEncoding =
	// Degree of the hypothesis (+ 1)
	"time(0..degree).\n"
	// Perform actions
	"1 {apply(A, T) : action(A)} 1 :- time(T), T > 0.\n"
	// Check preconditions
	":- apply(A, T), demands(A, F, true), not holds(F, T - 1), time(T), time(T - 1).\n"
	":- apply(A, T), demands(A, F, false), holds(F, T - 1), time(T), time(T - 1).\n"
	// Apply effects
	"holds(F, T) :- apply(A, T), adds(A, F), action(A), time(T).\n"
	"del(F, T) :- apply(A, T), deletes(A, F), action(A), time(T).\n"
	"holds(F, T) :- holds(F, T - 1), not del(F, T), time(T), time(T - 1).\n"
	// Eliminate all states not complying with the constraint at t = 0
	":- hypothesisConstraint(0).\n"
	// Eliminate all states complying with the constraint at t = 1
	":- not hypothesisConstraint(1).\n";

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

	mergeEncodings();

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

	// Remove constraints with a too high degree
	// TODO: reimplement

	// Remove all “terminal” literal
	// TODO: reimplement

	// Remove all constraints subsumed by previously proven constraints
	// TODO: reimplement

	// Sort in descending order so that we can efficiently pop elements from the back
	// TODO: reimplement

	while (!m_extractedConstraints.empty())
	{
		// TODO: don’t copy constraint
		auto candidate = *m_extractedConstraints.begin();
		m_extractedConstraints.erase(m_extractedConstraints.begin());

		m_claspConstraintLogger->fill(m_configuration->constraintsToExtract);

		if (m_environment->logLevel() == LogLevel::Debug)
			std::cout << "[Info ] Testing hypothesis (degree: " << candidate.degree()
				<< ", #literals: " << candidate.literals().size() << ")" << std::endl;

		auto proofResult = ProofResult::Unknown;

		switch (m_configuration->proofMethod)
		{
			case ProofMethod::StateWise:
				proofResult = testHypothesisStateWise(candidate, EventHypothesisTested::Purpose::Prove);
				break;
			case ProofMethod::Inductive:
				proofResult = testHypothesisInductively(candidate, EventHypothesisTested::Purpose::Prove);
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
				std::cout << "[Info ] \033[1;31mHypothesis unproven\033[0m" << std::endl;

			continue;
		}

		if (proofResult == ProofResult::GroundingTimeout || proofResult == ProofResult::SolvingTimeout)
		{
			if (m_environment->logLevel() == LogLevel::Debug)
				std::cout << "[Info ] \033[1;33mTimeout proving hypothesis\033[0m" << std::endl;

			continue;
		}

		if (m_configuration->minimizationStrategy == MinimizationStrategy::SimpleMinimization)
			candidate = minimizeConstraint(candidate, 0);
		else if (m_configuration->minimizationStrategy == MinimizationStrategy::LinearMinimization)
			candidate = minimizeConstraint(candidate, 1);

		// TODO: reimplement constraint subsumption

		m_provenConstraints.push_back(candidate);

		std::cout << "[Info ] \033[1;32mHypothesis proven\033[0m" << " ("
			<< m_provenConstraints.size() << "/"
			<< m_configuration->constraintsToProve << ")" << std::endl;

		// TODO: reimplement testing policies

		auto &directConstraintsStream = m_environment->directConstraintsStream();
		auto &generalizedConstraintsStream = m_environment->generalizedConstraintsStream();

		candidate.print(directConstraintsStream);
		directConstraintsStream << std::endl;

		candidate.printGeneralized(generalizedConstraintsStream, -candidate.timeRange().min);
		generalizedConstraintsStream << std::endl;

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

void FeedbackLoop::mergeEncodings()
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

	std::for_each(m_provenConstraints.cbegin(), m_provenConstraints.cend(),
		[&](const auto &learnedConstraint)
		{
			learnedConstraint.printGeneralized(metaEncoding, -learnedConstraint.timeRange().min);
			metaEncoding << std::endl;
		});

	metaEncoding.clear();
	metaEncoding.seekg(0, std::ios::beg);

	m_claspConstraintLogger = std::make_unique<ClaspConstraintLogger>(metaEncoding, m_extractedConstraints, *m_configuration);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Constraint FeedbackLoop::minimizeConstraint(const Constraint &provenGeneralizedConstraint, size_t linearIncrement)
{
	//const auto literalsBefore = provenGeneralizedConstraint.numberOfLiterals();
	//const auto degreeBefore = provenGeneralizedConstraint.degree();
	size_t requiredTests = 0;

	// If nothing works, keep the original constraint
	auto result = provenGeneralizedConstraint;

	if (m_environment->logLevel() == LogLevel::Debug)
		std::cout << "[Info ] Trying to minimize " << provenGeneralizedConstraint << std::endl;

	// Start with windows of size 1
	size_t windowSize = 1;

	for (size_t i = 0; i < result.literals().size();)
	{
		if (m_environment->logLevel() == LogLevel::Debug)
			std::cout << "[Info ] Trying to eliminate " << windowSize << " literals starting at " << i << std::endl;

		Constraint candidate = result.withoutLiterals(i, windowSize);

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
				proofResult = testHypothesisStateWise(candidate, EventHypothesisTested::Purpose::Minimize);
				break;
			case ProofMethod::Inductive:
				proofResult = testHypothesisInductively(candidate, EventHypothesisTested::Purpose::Minimize);
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
			<< (provenGeneralizedConstraint.literals().size() - result.literals().size())
			<< "/" << provenGeneralizedConstraint.literals().size()
			<< " literals through minimization\033[0m" << std::endl;
	}

	// TODO: reimplement statistics

	return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

ProofResult FeedbackLoop::testHypothesisStateWise(const Constraint &candidate, EventHypothesisTested::Purpose purpose)
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
		<< "hypothesisConstraint(T) ";

	candidate.printGeneralized(proofEncoding, -candidate.timeRange().min);

	proofEncoding
		<< std::endl
		<< StateWiseProofEncoding << std::endl;

	std::for_each(m_provenConstraints.cbegin(), m_provenConstraints.cend(),
		[&](const auto &constraint)
		{
			constraint.printGeneralized(proofEncoding, -constraint.timeRange().min);
			proofEncoding << std::endl;
		});

	proofEncoding.clear();
	proofEncoding.seekg(0, std::ios::beg);

	// TODO: add warning/error message handler
	// TODO: handle grounding/solving timeouts
	// TODO: make timeout configurable
	// TODO: record statistics, including grounding time
	Clingo::Control clingoControl{{"--stats=2"}};
	clingoControl.add("base", {}, proofEncoding.str().c_str());
	clingoControl.ground({{"base", {}}});

	auto satisfiable = Satisfiability::Unsatisfiable;

	for (auto model : clingoControl.solve_iteratively())
		satisfiable = Satisfiability::Satisfiable;

	if (satisfiable == Satisfiability::Unsatisfiable)
		return ProofResult::Proven;
	else
		return ProofResult::Unproven;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

ProofResult FeedbackLoop::testHypothesisInductively(const Constraint &candidate, EventHypothesisTested::Purpose purpose)
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
			<< "hypothesisConstraint(T) ";

		candidate.printGeneralized(proofEncoding, -candidate.timeRange().min);

		proofEncoding
			<< std::endl
			<< InductiveProofBaseEncoding << std::endl;

		std::for_each(m_provenConstraints.cbegin(), m_provenConstraints.cend(),
			[&](const auto &constraint)
			{
				constraint.printGeneralized(proofEncoding, -constraint.timeRange().min);
				proofEncoding << std::endl;
			});

		proofEncoding.clear();
		proofEncoding.seekg(0, std::ios::beg);

		// TODO: add warning/error message handler
		// TODO: handle grounding/solving timeouts
		// TODO: make timeout configurable
		// TODO: record statistics, including grounding time
		Clingo::Control clingoControl{{"--stats=2"}};
		clingoControl.add("base", {}, proofEncoding.str().c_str());
		clingoControl.ground({{"base", {}}});

		auto satisfiable = Satisfiability::Unsatisfiable;

		for (auto model : clingoControl.solve_iteratively())
			satisfiable = Satisfiability::Satisfiable;

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
			<< "hypothesisConstraint(T) ";

		candidate.printGeneralized(proofEncoding, -candidate.timeRange().min);

		proofEncoding
			<< std::endl
			<< InductiveProofStepEncoding << std::endl;

		std::for_each(m_provenConstraints.cbegin(), m_provenConstraints.cend(),
			[&](const auto &constraint)
			{
				constraint.printGeneralized(proofEncoding, -constraint.timeRange().min);
				proofEncoding << std::endl;
			});

		proofEncoding.clear();
		proofEncoding.seekg(0, std::ios::beg);

		// TODO: add warning/error message handler
		// TODO: handle grounding/solving timeouts
		// TODO: make timeout configurable
		// TODO: record statistics, including grounding time
		Clingo::Control clingoControl{{"--stats=2"}};
		clingoControl.add("base", {}, proofEncoding.str().c_str());
		clingoControl.ground({{"base", {}}});

		auto satisfiable = Satisfiability::Unsatisfiable;

		for (auto model : clingoControl.solve_iteratively())
			satisfiable = Satisfiability::Satisfiable;

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
