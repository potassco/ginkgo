#include <ginkgo/feedback-loop/production/FeedbackLoop.h>

#include <iostream>
#include <limits>
#include <thread>

#include <clingo.hh>
#include <clasp/clasp_facade.h>

#include <json/json.h>

#include <ginkgo/feedback-loop/production/Minimization.h>
#include <ginkgo/feedback-loop/production/PartialPrograms.h>
#include <ginkgo/feedback-loop/production/ProofInductive.h>
#include <ginkgo/feedback-loop/production/ProofStateWise.h>
#include <ginkgo/solving/Satisfiability.h>
#include <ginkgo/utils/TextFile.h>

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
				proofResult = testCandidateStateWise(candidate, m_program, *m_configuration);
				break;
			case ProofMethod::Inductive:
				proofResult = testCandidateInductively(candidate, m_program, *m_configuration);
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
			candidate = minimizeConstraint(candidate, 0, m_program, *m_configuration, *m_environment);
		else if (m_configuration->minimizationStrategy == MinimizationStrategy::LinearMinimization)
			candidate = minimizeConstraint(candidate, 1, m_program, *m_configuration, *m_environment);

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

}
}
}
