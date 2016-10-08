#include <ginkgo/feedback-loop/production/FeedbackLoop.h>

#include <iostream>
#include <limits>
#include <thread>

#include <json/json.h>

#include <ginkgo/utils/TextFile.h>
#include <ginkgo/solving/ClaspOutputParsing.h>

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

FeedbackLoop::FeedbackLoop(std::unique_ptr<Environment> environment, std::unique_ptr<Configuration<Plain> > configuration)
:	m_environment(std::move(environment)),
	m_configuration(std::move(configuration)),
	m_gringo(m_environment->gringoConfiguration()),
	m_clasp(m_environment->claspConfiguration()),
	m_xclasp(m_environment->xclaspConfiguration()),
	m_feedback(m_environment->symbolTable()),
	m_learnedConstraints(m_environment->symbolTable())
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////

FeedbackLoop::~FeedbackLoop()
{
	if (m_xclasp.isRunning())
	{
		m_xclasp.resume();
		m_xclasp.terminate();
		m_xclasp.join();
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void FeedbackLoop::run()
{
	setlocale(LC_NUMERIC, "C");

	m_feedback.clear();

	mergeEncodings();

	m_events.startTimer();

	bool startOver = true;

	while (true)
	{
		generateFeedback(m_configuration->constraintsToExtract, startOver);

		if (m_feedback.empty())
			// No more constraints, exiting
			break;

		// If we couldn't prove anything, we'll extract more constraints next time
		startOver = false;

		auto extractedConstraints = m_feedback.size();

		// Remove constraints with a too high degree
		m_feedback.removeConstraintsWithTooHighDegree(m_configuration->maxDegree);

		if (m_environment->logLevel() == LogLevel::Debug)
		{
			std::cout << "[Info ] \033[1;33mRemoved " << (extractedConstraints - m_feedback.size())
				<< " constraints with a degree higher than " << m_configuration->maxDegree
				<< "\033[0m" << std::endl;
		}

		// Statistics
		{
			EventConstraintsRemoved event =
			{
				EventConstraintsRemoved::Source::Feedback,
				EventConstraintsRemoved::Reason::DegreeTooHigh,
				extractedConstraints - m_feedback.size(),
				m_feedback.size()
			};

			m_events.notifyConstraintsRemoved(event);
		}

		extractedConstraints = m_feedback.size();

		// Remove constraints with too many literals
		m_feedback.removeConstraintsContainingTooManyLiterals(m_configuration->maxNumberOfLiterals);

		if (m_environment->logLevel() == LogLevel::Debug)
		{
			std::cout << "[Info ] \033[1;33mRemoved " << (extractedConstraints - m_feedback.size())
				<< " constraints containing more than " << m_configuration->maxNumberOfLiterals
				<< " literals\033[0m" << std::endl;
		}

		// Statistics
		{
			EventConstraintsRemoved event =
			{
				EventConstraintsRemoved::Source::Feedback,
				EventConstraintsRemoved::Reason::ContainsTooManyLiterals,
				extractedConstraints - m_feedback.size(),
				m_feedback.size()
			};

			m_events.notifyConstraintsRemoved(event);
		}

		extractedConstraints = m_feedback.size();

		// Remove all 'terminal' literal
		m_feedback.removeLiterals("terminal");

		// Remove all constraints subsumed by previously proven constraints
		std::for_each(m_learnedConstraints.cbegin(), m_learnedConstraints.cend(), [&](auto constraint)
		{
			m_feedback.removeConstraintsSubsumedBy(GeneralizedConstraint(constraint));
		});

		// Statistics
		{
			EventConstraintsRemoved event =
			{
				EventConstraintsRemoved::Source::Feedback,
				EventConstraintsRemoved::Reason::Subsumed,
				extractedConstraints - m_feedback.size(),
				m_feedback.size()
			};

			m_events.notifyConstraintsRemoved(event);
		}

		// Sort in descending order so that we can efficiently pop elements from the back
		m_feedback.sortBy(Constraints::SortKey::TimeDegree, Constraints::SortDirection::Descending, true);

		std::for_each(m_feedback.cbegin(), m_feedback.cend(), [](const auto &constraint)
		{
			BOOST_ASSERT(constraint);
		});

		while (!m_feedback.empty())
		{
			const auto constraint = m_feedback.back();
			// Pop the constraint to test
			m_feedback.pop_back();

			BOOST_ASSERT(!constraint->containsIdentifier("terminal"));

			auto hypothesis = GeneralizedConstraint(constraint);

			if (m_environment->logLevel() == LogLevel::Debug)
			{
				std::cout << "[Info ] Testing hypothesis (degree: " << hypothesis.degree()
					<< ", #literals: " << hypothesis.numberOfLiterals() << ")" << std::endl;
			}

			auto proofResult = ProofResult::Unknown;

			switch (m_configuration->proofMethod)
			{
				case ProofMethod::StateWise:
					proofResult = testHypothesisStateWise(hypothesis, EventHypothesisTested::Purpose::Prove);
					break;
				case ProofMethod::Inductive:
					proofResult = testHypothesisInduction(hypothesis, EventHypothesisTested::Purpose::Prove);
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

			// TODO: Handle sigterm etc. and gracefully terminate all child processes

			// Constraint is proven, now try to minimize it (if enabled)
			if (m_configuration->minimizationStrategy == MinimizationStrategy::SimpleMinimization)
				hypothesis = minimizeConstraint(hypothesis, 0);
			else if (m_configuration->minimizationStrategy == MinimizationStrategy::LinearMinimization)
				hypothesis = minimizeConstraint(hypothesis, 1);

			// If proven, remove all subsumed constraints (they are weaker and also satisfied)
			const auto feedbackSizeBefore = m_feedback.size();
			m_feedback.removeConstraintsSubsumedBy(hypothesis);

			if (m_environment->logLevel() == LogLevel::Debug)
				std::cout << "[Info ] New constraint subsumed " << (feedbackSizeBefore - m_feedback.size()) << " constraints from feedback" << std::endl;

			// Statistics
			{
				EventConstraintsRemoved event =
				{
					EventConstraintsRemoved::Source::Feedback,
					EventConstraintsRemoved::Reason::Subsumed,
					(feedbackSizeBefore - m_feedback.size()),
					m_feedback.size()
				};

				m_events.notifyConstraintsRemoved(event);
			}

			std::for_each(m_feedback.cbegin(), m_feedback.cend(), [](auto constraint)
			{
				BOOST_ASSERT(constraint);
			});

			// Add new generalized constraint
			m_learnedConstraints.push_back(hypothesis.originalConstraint());

			auto &directConstraintsStream = m_environment->directConstraintsStream();
			auto &generalizedConstraintsStream = m_environment->generalizedConstraintsStream();

			constraint->print(directConstraintsStream);
			directConstraintsStream << std::endl;

			hypothesis.print(generalizedConstraintsStream);
			generalizedConstraintsStream << std::endl;

			// Statistics
			{
				EventConstraintLearned event =
				{
					hypothesis.degree(),
					hypothesis.numberOfLiterals(),
					m_learnedConstraints.size()
				};

				m_events.notifyConstraintLearned(event);
			}

			std::cout << "[Info ] \033[1;32mHypothesis proven\033[0m" << " ("
				<< m_learnedConstraints.size() << "/"
				<< m_configuration->constraintsToProve << ")" << std::endl;

			// We have learned a constraint, so we can extract new feedback next time
			startOver = true;

			// Generate new feedback when using the find-first policy (else, continue testing with test-all)
			if (m_configuration->testingPolicy == TestingPolicy::FindFirst)
				break;

			// Stop if we have proven enough constraints
			if (m_learnedConstraints.size() >= m_configuration->constraintsToProve)
				break;
		}

		// Stop if we have proven enough constraints
		if (m_learnedConstraints.size() >= m_configuration->constraintsToProve)
			break;
	}

	if (m_xclasp.isRunning())
	{
		m_xclasp.resume();
		m_xclasp.terminate();
		m_xclasp.join();
	}

	// Statistics
	if (m_learnedConstraints.size() == m_configuration->constraintsToProve)
	{
		EventFinished event = {EventFinished::Reason::Done};
		m_events.notifyFinished(event);
	}

	auto &statisticsStream = m_environment->statisticsStream();

	Json::Value statistics;
	statistics["Configuration"] = m_configuration->toJSON();
	statistics["Events"] = m_events.toJSON();

	statisticsStream << statistics;
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

void FeedbackLoop::generateFeedback(size_t constraintsToExtract, bool startOver)
{
	if (startOver)
	{
		m_program.clear();
		m_program.seekg(0, std::ios::beg);

		std::stringstream metaEncoding;
		metaEncoding
			<< "#const horizon=" << m_configuration->horizon << "." << std::endl
			<< MetaEncoding << std::endl
			<< m_program.rdbuf() << std::endl;

		std::for_each(m_learnedConstraints.cbegin(), m_learnedConstraints.cend(), [&](const auto &constraint)
		{
			GeneralizedConstraint(constraint).print(metaEncoding);
			metaEncoding << std::endl;
		});

		metaEncoding.clear();
		metaEncoding.seekg(0, std::ios::beg);

		bool extractionTimeout = false;

		m_gringo.run(metaEncoding, m_configuration->extractionTimeout, extractionTimeout);
		m_gringo.join();

		if (m_xclasp.isRunning())
		{
			m_xclasp.resume();
			m_xclasp.terminate();
			m_xclasp.join();

			if (m_environment->logLevel() == LogLevel::Debug)
				std::cout << "[Info ] Terminated feedback extraction" << std::endl;
		}

		m_feedback.clear();

		if (extractionTimeout)
		{
			// Statistics
			const EventFinished event = {EventFinished::Reason::ExtractionTimeout};
			m_events.notifyFinished(event);
			std::cerr << "[Info ] Knowledge extraction timeout, exiting" << std::endl;
			return;
		}
	}

	// TODO: Clean up symbol table at some point
	if (startOver)
	{
		BOOST_ASSERT(m_gringo.stdout());
		m_xclasp.run(*m_gringo.stdout(), false, true);

		if (m_environment->logLevel() == LogLevel::Debug)
			std::cout << "[Info ] Starting feedback extraction" << std::endl;
	}
	else
	{
		if (!m_xclasp.isRunning())
		{
			// Statistics
			const EventFinished event = {EventFinished::Reason::FeedbackEmpty};
			m_events.notifyFinished(event);
			std::cerr << "[Info ] No more constraints available (solver terminated) ..." << std::endl;
			return;
		}

		m_xclasp.resume();

		if (m_environment->logLevel() == LogLevel::Debug)
			std::cout << "[Info ] Resuming feedback extraction" << std::endl;
	}

	const auto startTime = std::chrono::high_resolution_clock::now();

	// Extract requested number of constraints
	while (true)
	{
		if (!m_xclasp.waitForEvent())
		{
			// Clasp terminated; join thread
			m_xclasp.join();
			break;
		}

		std::lock_guard<std::mutex> lock(m_xclasp.stderrAccessMutex());

		auto *stderr = m_xclasp.stderr();
		if (!stderr)
			continue;

		const auto constraintString = stderr->str();
		m_xclasp.clearStderr();

		if (constraintString.size() < 2)
			continue;

		// TODO: Don't copy stderr
		auto constraint = std::make_shared<Constraint>(m_feedback.size(), constraintString, m_environment->symbolTable());
		m_feedback.push_back(std::move(constraint));

		if (m_environment->logLevel() == LogLevel::Debug)
		{
			if (m_feedback.size() % 256 == 0)
				std::cout << "[Debug] " << m_feedback.size() << " constraints" << std::endl;
		}

		if (m_feedback.size() >= constraintsToExtract)
			break;
	}

	if (m_xclasp.exists())
		m_xclasp.pause();

	if (m_environment->logLevel() == LogLevel::Debug)
		std::cout << "[Info ] Extracted " << m_feedback.size() << " constraints" << std::endl;

	const auto now = std::chrono::high_resolution_clock::now();

	EventFeedbackExtracted event =
	{
		startOver ? EventFeedbackExtracted::Mode::StartOver : EventFeedbackExtracted::Mode::Resume,
		std::chrono::duration<double>(now - startTime).count(),
		constraintsToExtract,
		m_feedback.size()
	};

	m_events.notifyFeedbackExtracted(event);

	if (m_feedback.empty())
	{
		// Statistics
		const EventFinished event = {EventFinished::Reason::FeedbackEmpty};
		m_events.notifyFinished(event);
		std::cerr << "[Info ] No more constraints available (solver didn't find anything) ..." << std::endl;
		return;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////

GeneralizedConstraint FeedbackLoop::minimizeConstraint(const GeneralizedConstraint &provenGeneralizedConstraint, size_t linearIncrement)
{
	const auto literalsBefore = provenGeneralizedConstraint.numberOfLiterals();
	const auto degreeBefore = provenGeneralizedConstraint.degree();
	size_t requiredTests = 0;

	// If nothing works, keep the original constraint
	GeneralizedConstraint result = provenGeneralizedConstraint;

	if (m_environment->logLevel() == LogLevel::Debug)
		std::cout << "[Info ] Trying to minimize " << provenGeneralizedConstraint << std::endl;

	// Start with windows of size 1
	size_t windowSize = 1;

	for (size_t i = 0; i < result.numberOfLiterals();)
	{
		if (m_environment->logLevel() == LogLevel::Debug)
			std::cout << "[Info ] Trying to eliminate " << windowSize << " literals starting at " << i << std::endl;

		GeneralizedConstraint hypothesis(result.originalConstraint()->withoutLiterals(i, windowSize));

		// Skip candidates that have become empty due to removing literals
		if (hypothesis.numberOfLiterals() == 0)
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
				proofResult = testHypothesisStateWise(hypothesis, EventHypothesisTested::Purpose::Minimize);
				break;
			case ProofMethod::Inductive:
				proofResult = testHypothesisInduction(hypothesis, EventHypothesisTested::Purpose::Minimize);
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
		result = hypothesis;

		// Also increase the window size by 1
		windowSize += linearIncrement;
	}

	if (m_environment->logLevel() == LogLevel::Debug)
	{
		std::cout << "[Info ] \033[1;34mEliminated "
			<< (provenGeneralizedConstraint.numberOfLiterals() - result.numberOfLiterals())
			<< "/" << provenGeneralizedConstraint.numberOfLiterals()
			<< " literals through minimization\033[0m" << std::endl;
	}

	// Statistics
	{
		const auto literalsAfter = result.numberOfLiterals();
		const auto degreeAfter = result.degree();

		EventMinimized event =
		{
			literalsBefore - literalsAfter,
			result.numberOfLiterals(),
			degreeBefore - degreeAfter,
			degreeAfter,
			requiredTests
		};

		m_events.notifyMinimized(event);
	}

	return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

ProofResult FeedbackLoop::testHypothesisStateWise(const GeneralizedConstraint &generalizedHypothesis, EventHypothesisTested::Purpose purpose)
{
	m_program.clear();
	m_program.seekg(0, std::ios::beg);

	std::stringstream proofEncoding;
	proofEncoding
		<< m_program.rdbuf();

	if (m_configuration->fluentClosureUsage == FluentClosureUsage::UseFluentClosure)
		proofEncoding << FluentClosureEncoding;
	else
		proofEncoding << StateGeneratorEncoding;

	proofEncoding
		<< "#const degree=" << generalizedHypothesis.degree() << "." << std::endl
		<< "hypothesisConstraint(T) " << generalizedHypothesis << std::endl
		<< StateWiseProofEncoding << std::endl;

	std::for_each(m_learnedConstraints.cbegin(), m_learnedConstraints.cend(), [&](const auto &constraint)
	{
		GeneralizedConstraint(constraint).print(proofEncoding);
		proofEncoding << std::endl;
	});

	proofEncoding.clear();
	proofEncoding.seekg(0, std::ios::beg);

	bool groundingTimeout = false;
	bool solvingTimeout = false;

	const auto groundingStartTime = std::chrono::high_resolution_clock::now();
	m_gringo.run(proofEncoding, m_configuration->hypothesisTestingTimeout, groundingTimeout);
	m_gringo.join();
	const auto groundingFinishedTime = std::chrono::high_resolution_clock::now();

	auto satisfiable = Satisfiability::Unknown;

	if (!groundingTimeout)
	{
		BOOST_ASSERT(m_gringo.stdout());

		if (m_gringo.stderr() && parseForWarnings(*m_gringo.stderr()))
			std::cout << "[Warn ] Warning while grounding: " << m_gringo.stderr()->rdbuf() << std::endl;

		m_clasp.run(*m_gringo.stdout(), m_configuration->hypothesisTestingTimeout, solvingTimeout);
		m_clasp.join();

		BOOST_ASSERT(m_clasp.stdout());
		satisfiable = parseForSatisfiability(*m_clasp.stdout());
	}

	auto proofResult = ProofResult::Unknown;

	if (groundingTimeout)
		proofResult = ProofResult::GroundingTimeout;
	else if (solvingTimeout)
		proofResult = ProofResult::SolvingTimeout;
	else if (satisfiable == Satisfiability::Unsatisfiable)
		proofResult = ProofResult::Proven;
	else if (satisfiable == Satisfiability::Satisfiable)
		proofResult = ProofResult::Unproven;
	else
		std::cout << "[Warn ] Proof result is unknown" << std::endl;

	// Statistics
	{
		EventHypothesisTested event =
		{
			ProofType::StateWise,
			purpose,
			generalizedHypothesis.degree(),
			generalizedHypothesis.numberOfLiterals(),
			proofResult,
			std::chrono::duration<double>(groundingFinishedTime - groundingStartTime).count()
		};
		*m_clasp.stdout() >> event.claspJSONOutput;

		m_events.notifyHypothesisTested(event);
	}

	return proofResult;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

ProofResult FeedbackLoop::testHypothesisInduction(const GeneralizedConstraint &generalizedHypothesis, EventHypothesisTested::Purpose purpose)
{
	m_program.clear();
	m_program.seekg(0, std::ios::beg);

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Induction Base
	////////////////////////////////////////////////////////////////////////////////////////////////
	{
		std::stringstream inductionBaseEncoding;
		inductionBaseEncoding
			<< m_program.rdbuf()
			<< "#const degree=" << generalizedHypothesis.degree() << "." << std::endl
			<< "hypothesisConstraint(T) ";

		generalizedHypothesis.print(inductionBaseEncoding);

		inductionBaseEncoding
			<< std::endl
			<< InductiveProofBaseEncoding << std::endl;

		std::for_each(m_learnedConstraints.cbegin(), m_learnedConstraints.cend(), [&](const auto &constraint)
		{
			GeneralizedConstraint(constraint).print(inductionBaseEncoding);
			inductionBaseEncoding << std::endl;
		});

		inductionBaseEncoding.clear();
		inductionBaseEncoding.seekg(0, std::ios::beg);

		bool groundingTimeout = false;
		bool solvingTimeout = false;

		const auto groundingStartTime = std::chrono::high_resolution_clock::now();
		m_gringo.run(inductionBaseEncoding, m_configuration->hypothesisTestingTimeout, groundingTimeout);
		m_gringo.join();
		const auto groundingFinishedTime = std::chrono::high_resolution_clock::now();

		auto satisfiable = Satisfiability::Unknown;

		if (!groundingTimeout)
		{
			BOOST_ASSERT(m_gringo.stdout());

			if (m_gringo.stderr() && parseForWarnings(*m_gringo.stderr()))
				std::cout << "[Warn ] Warning while grounding: " << m_gringo.stderr()->rdbuf() << std::endl;

			m_clasp.run(*m_gringo.stdout(), m_configuration->hypothesisTestingTimeout, solvingTimeout);
			m_clasp.join();

			BOOST_ASSERT(m_clasp.stdout());
			satisfiable = parseForSatisfiability(*m_clasp.stdout());
		}

		auto proofResult = ProofResult::Unknown;

		if (groundingTimeout)
			proofResult = ProofResult::GroundingTimeout;
		else if (solvingTimeout)
			proofResult = ProofResult::SolvingTimeout;
		else if (satisfiable == Satisfiability::Unsatisfiable)
			proofResult = ProofResult::Proven;
		else
			proofResult = ProofResult::Unproven;

		// Statistics
		{
			EventHypothesisTested event =
			{
				ProofType::InductiveBase,
				purpose,
				generalizedHypothesis.degree(),
				generalizedHypothesis.numberOfLiterals(),
				proofResult,
				std::chrono::duration<double>(groundingFinishedTime - groundingStartTime).count()
			};
			*m_clasp.stdout() >> event.claspJSONOutput;

			m_events.notifyHypothesisTested(event);
		}

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
		std::stringstream inductionStepEncoding;
		inductionStepEncoding
			<< m_program.rdbuf();

		if (m_configuration->fluentClosureUsage == FluentClosureUsage::UseFluentClosure)
			inductionStepEncoding << FluentClosureEncoding;
		else
			inductionStepEncoding << StateGeneratorEncoding;

		inductionStepEncoding
			<< "#const degree=" << (generalizedHypothesis.degree() + 1) << "." << std::endl
			<< "hypothesisConstraint(T) ";

		generalizedHypothesis.print(inductionStepEncoding);

		inductionStepEncoding
			<< std::endl
			<< InductiveProofStepEncoding << std::endl;

		std::for_each(m_learnedConstraints.cbegin(), m_learnedConstraints.cend(), [&](const auto &constraint)
		{
			GeneralizedConstraint(constraint).print(inductionStepEncoding);
			inductionStepEncoding << std::endl;
		});

		inductionStepEncoding.clear();
		inductionStepEncoding.seekg(0, std::ios::beg);

		bool groundingTimeout = false;
		bool solvingTimeout = false;

		const auto groundingStartTime = std::chrono::high_resolution_clock::now();
		m_gringo.run(inductionStepEncoding, m_configuration->hypothesisTestingTimeout, groundingTimeout);
		m_gringo.join();
		const auto groundingFinishedTime = std::chrono::high_resolution_clock::now();

		auto satisfiable = Satisfiability::Unknown;

		if (!groundingTimeout)
		{
			BOOST_ASSERT(m_gringo.stdout());

			if (m_gringo.stderr() && parseForWarnings(*m_gringo.stderr()))
				std::cout << "[Warn ] Warning while grounding: " << m_gringo.stderr()->rdbuf() << std::endl;

			m_clasp.run(*m_gringo.stdout(), m_configuration->hypothesisTestingTimeout, solvingTimeout);
			m_clasp.join();

			BOOST_ASSERT(m_clasp.stdout());
			satisfiable = parseForSatisfiability(*m_clasp.stdout());
		}

		auto proofResult = ProofResult::Unknown;

		if (groundingTimeout)
			proofResult = ProofResult::GroundingTimeout;
		else if (solvingTimeout)
			proofResult = ProofResult::SolvingTimeout;
		else if (satisfiable == Satisfiability::Unsatisfiable)
			proofResult = ProofResult::Proven;
		else
			proofResult = ProofResult::Unproven;

		// Statistics
		{
			EventHypothesisTested event =
			{
				ProofType::InductiveStep,
				purpose,
				generalizedHypothesis.degree(),
				generalizedHypothesis.numberOfLiterals(),
				proofResult,
				std::chrono::duration<double>(groundingFinishedTime - groundingStartTime).count()
			};
			*m_clasp.stdout() >> event.claspJSONOutput;

			m_events.notifyHypothesisTested(event);
		}

		return proofResult;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////

}
}
}
