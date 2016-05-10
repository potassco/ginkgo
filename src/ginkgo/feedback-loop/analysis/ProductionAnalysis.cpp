#include <ginkgo/feedback-loop/analysis/ProductionAnalysis.h>

#include <fstream>
#include <iostream>
#include <json/json.h>
#include <json/value.h>
#include <boost/assert.hpp>

#include <ginkgo/utils/Statistics.h>
#include <ginkgo/utils/Utils.h>

namespace ginkgo
{
namespace feedbackLoop
{
namespace analysis
{

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// ProductionAnalysis
//
////////////////////////////////////////////////////////////////////////////////////////////////////

ProductionAnalysis<Plain> ProductionAnalysis<Plain>::fromRawFile(const boost::filesystem::path &path, double penalty)
{
	ProductionAnalysis<Plain> productionAnalysis;

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Read file
	////////////////////////////////////////////////////////////////////////////////////////////////

	setlocale(LC_NUMERIC, "C");

	if (!boost::filesystem::is_regular_file(path))
		std::cerr << "[Error] File does not exist: " << path.string() << std::endl;

	Json::Value productionStatistics;

	try
	{
		// Read production statistics file
		std::cout << "[Info ] Reading production statistics from " << path.string() << std::endl;
		std::ifstream statisticsStream(path.string(), std::ios::in);

		statisticsStream >> productionStatistics;
	}
	catch (std::exception &e)
	{
		std::cerr << "[Error] " << e.what() << std::endl;
	}

	auto productionEvents = production::Events::fromJSON(productionStatistics["Events"]);

	// TODO: Remove legacy support
	if (productionStatistics.get("Configuration", Json::nullValue) != Json::nullValue)
		productionAnalysis.configuration = production::Configuration<Plain>::fromJSON(productionStatistics["Configuration"]);
	else
		productionAnalysis.configuration = production::Configuration<Plain>::fromJSON(productionStatistics["Environment"]);

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Run Analysis (Production)
	////////////////////////////////////////////////////////////////////////////////////////////////

	BOOST_ASSERT(penalty > 0.0);

	const auto &timedEventFinished = productionEvents.eventFinished();
	const auto &timedEventsHypothesisTested = productionEvents.eventsHypothesisTested();
	const auto &timedEventsFeedbackExtracted = productionEvents.eventsFeedbackExtracted();
	const auto &timedEventsMinimized = productionEvents.eventsMinimized();
	const auto &timedEventsConstraintsRemoved = productionEvents.eventsConstraintsRemoved();

	const auto &eventFinishedTime = std::get<0>(timedEventFinished);
	const auto &eventFinished = std::get<1>(timedEventFinished);

	productionAnalysis.runtime = eventFinishedTime;
	productionAnalysis.finishedNormally = eventFinished.reason == production::EventFinished::Reason::Done;

	const auto purposeProve = production::EventHypothesisTested::Purpose::Prove;
	const auto purposeMinimize = production::EventHypothesisTested::Purpose::Minimize;

	std::for_each(timedEventsHypothesisTested.cbegin(), timedEventsHypothesisTested.cend(),
		[&](const auto &timedEvent)
		{
			const auto &event = std::get<1>(timedEvent);

			if (event.proofType == production::ProofType::StateWiseProof || event.proofType == production::ProofType::InductionBaseProof)
			{
				if (event.purpose == purposeProve)
				{
					productionAnalysis.proofs++;

					productionAnalysis.hypothesisDegreeMin = std::min(productionAnalysis.hypothesisDegreeMin, event.hypothesisDegree);
					productionAnalysis.hypothesisDegreeTotal += event.hypothesisDegree;
					productionAnalysis.hypothesisDegreeMax = std::max(productionAnalysis.hypothesisDegreeMax, event.hypothesisDegree);

					productionAnalysis.hypothesisLiteralsMin = std::min(productionAnalysis.hypothesisLiteralsMin, event.hypothesisLiterals);
					productionAnalysis.hypothesisLiteralsTotal += event.hypothesisLiterals;
					productionAnalysis.hypothesisLiteralsMax = std::max(productionAnalysis.hypothesisLiteralsMax, event.hypothesisLiterals);

					productionAnalysis.hypothesesTested++;
				}
				else if (event.purpose == purposeMinimize)
					productionAnalysis.minimizationProofs++;
			}

			if ((event.proofType == production::ProofType::StateWiseProof || event.proofType == production::ProofType::InductionStepProof)
				&& event.proofResult == production::ProofResult::Proven)
			{
				if (event.purpose == purposeProve)
					productionAnalysis.proofsSuccessful++;
				else if (event.purpose == purposeMinimize)
					productionAnalysis.minimizationProofsSuccessful++;
			}

			if (event.purpose == purposeProve)
			{
				productionAnalysis.proofTimeGroundingTotal += event.groundingTime;
				productionAnalysis.proofTimeSolvingTotal += event.claspJSONOutput["Time"]["Total"].asDouble();
			}
			else if (event.purpose == purposeMinimize)
			{
				productionAnalysis.minimizationProofTimeGroundingTotal += event.groundingTime;
				productionAnalysis.minimizationProofTimeSolvingTotal += event.claspJSONOutput["Time"]["Total"].asDouble();
			}
		});

	// Info about skipped hypotheses
	std::for_each(timedEventsConstraintsRemoved.cbegin(), timedEventsConstraintsRemoved.cend(),
		[&](const auto &timedEvent)
		{
			const auto &event = std::get<1>(timedEvent);

			if (event.source != production::EventConstraintsRemoved::Source::Feedback)
				return;

			if (event.reason == production::EventConstraintsRemoved::Reason::ContainsTerminalLiteral)
				productionAnalysis.hypothesesSkippedContainsTerminalLiteral += event.removedConstraints;
			else if (event.reason == production::EventConstraintsRemoved::Reason::DegreeTooHigh)
				productionAnalysis.hypothesesSkippedDegreeTooHigh += event.removedConstraints;
			else if (event.reason == production::EventConstraintsRemoved::Reason::ContainsTooManyLiterals)
				productionAnalysis.hypothesesSkippedContainsTooManyLiterals += event.removedConstraints;
			else if (event.reason == production::EventConstraintsRemoved::Reason::Subsumed)
				productionAnalysis.hypothesesSkippedSubsumed += event.removedConstraints;
		});

	// Info about feedback extraction
	std::for_each(timedEventsFeedbackExtracted.begin(), timedEventsFeedbackExtracted.end(),
		[&](const auto &timedEvent)
		{
			const auto &event = std::get<1>(timedEvent);

			productionAnalysis.feedbackExtractionTimeTotal += event.duration;
			productionAnalysis.feedbackExtractionConstraintsTotal += event.extractedConstraints;

			productionAnalysis.feedbackExtractionResumes += (event.mode == production::EventFeedbackExtracted::Mode::Resume);
			productionAnalysis.feedbackExtractionRestarts += (event.mode == production::EventFeedbackExtracted::Mode::StartOver);
		});

	// Count literals removed by minimization
	std::for_each(timedEventsMinimized.begin(), timedEventsMinimized.end(),
		[&](const auto &timedEvent)
		{
			const auto &event = std::get<1>(timedEvent);

			// TODO: Make naming consistent
			productionAnalysis.minimizationLiteralsTotal += event.remainingLiterals + event.removedLiterals;
			productionAnalysis.minimizationLiteralsRemoved += event.removedLiterals;
			productionAnalysis.minimizationTests += event.requiredTests;
		});

	productionAnalysis.penalty = penalty;

	return productionAnalysis;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

ProductionAnalysis<Plain> ProductionAnalysis<Plain>::fromAnalysisFile(const boost::filesystem::path &path)
{
	setlocale(LC_NUMERIC, "C");

	std::cout << "[Info ] Reading analysis from " << path.string() << std::endl;

	Json::Value json;
	std::fstream file(path.string(), std::ios::in);
	file >> json;

	ProductionAnalysis productionAnalysis;

	const auto &jsonConfiguration = json["Configuration"];

	productionAnalysis.runtime = json["Runtime"].asDouble();
	productionAnalysis.finishedNormally = json["FinishedNormally"].asBool();

	productionAnalysis.proofs = static_cast<size_t>(json["Proofs"].asUInt64());
	productionAnalysis.proofsSuccessful = static_cast<size_t>(json["ProofsSuccessful"].asUInt64());
	productionAnalysis.proofTimeGroundingTotal = json["ProofTimeGroundingTotal"].asDouble();
	productionAnalysis.proofTimeSolvingTotal = json["ProofTimeSolvingTotal"].asDouble();

	productionAnalysis.minimizationProofs = static_cast<size_t>(json["MinimizationProofs"].asUInt64());
	productionAnalysis.minimizationProofsSuccessful = static_cast<size_t>(json["MinimizationProofsSuccessful"].asUInt64());
	productionAnalysis.minimizationProofTimeGroundingTotal = json["MinimizationProofTimeGroundingTotal"].asDouble();
	productionAnalysis.minimizationProofTimeSolvingTotal = json["MinimizationProofTimeSolvingTotal"].asDouble();

	productionAnalysis.minimizationLiteralsTotal = static_cast<size_t>(json["MinimizationLiteralsTotal"].asUInt64());
	productionAnalysis.minimizationLiteralsRemoved = static_cast<size_t>(json["MinimizationLiteralsRemoved"].asUInt64());
	productionAnalysis.minimizationTests = static_cast<size_t>(json["MinimizationTests"].asUInt64());

	productionAnalysis.hypothesisDegreeMin = static_cast<size_t>(json["HypothesisDegreeMin"].asUInt64());
	productionAnalysis.hypothesisDegreeTotal = static_cast<size_t>(json["HypothesisDegreeTotal"].asUInt64());
	productionAnalysis.hypothesisDegreeMax = static_cast<size_t>(json["HypothesisDegreeMax"].asUInt64());

	productionAnalysis.hypothesisLiteralsMin = static_cast<size_t>(json["HypothesisLiteralsMin"].asUInt64());
	productionAnalysis.hypothesisLiteralsTotal = static_cast<size_t>(json["HypothesisLiteralsTotal"].asUInt64());
	productionAnalysis.hypothesisLiteralsMax = static_cast<size_t>(json["HypothesisLiteralsMax"].asUInt64());

	productionAnalysis.hypothesesTested = static_cast<size_t>(json["HypothesesTested"].asUInt64());
	productionAnalysis.hypothesesSkippedContainsTerminalLiteral = static_cast<size_t>(json["HypothesesSkippedContainsTerminalLiteral"].asUInt64());
	productionAnalysis.hypothesesSkippedDegreeTooHigh = static_cast<size_t>(json["HypothesesSkippedDegreeTooHigh"].asUInt64());
	productionAnalysis.hypothesesSkippedContainsTooManyLiterals = static_cast<size_t>(json["HypothesesSkippedContainsTooManyLiterals"].asUInt64());
	productionAnalysis.hypothesesSkippedSubsumed = static_cast<size_t>(json["HypothesesSkippedSubsumed"].asUInt64());

	productionAnalysis.feedbackExtractionTimeTotal = static_cast<size_t>(json["FeedbackExtractionTimeTotal"].asUInt64());
	productionAnalysis.feedbackExtractionConstraintsTotal = static_cast<size_t>(json["FeedbackExtractionConstraintsTotal"].asUInt64());
	productionAnalysis.feedbackExtractionResumes = static_cast<size_t>(json["FeedbackExtractionResumes"].asUInt64());
	productionAnalysis.feedbackExtractionRestarts = static_cast<size_t>(json["FeedbackExtractionRestarts"].asUInt64());

	productionAnalysis.penalty = json["Penalty"].asDouble();

	productionAnalysis.configuration = production::Configuration<Plain>::fromJSON(jsonConfiguration);

	return productionAnalysis;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

ProductionAnalysis<Plain>::ProductionAnalysis()
{
	runtime = 0;
	finishedNormally = false;

	proofs = 0;
	proofsSuccessful = 0;
	proofTimeGroundingTotal = 0.0;
	proofTimeSolvingTotal = 0.0;

	minimizationProofs = 0;
	minimizationProofsSuccessful = 0;
	minimizationProofTimeGroundingTotal = 0.0;
	minimizationProofTimeSolvingTotal = 0.0;

	minimizationLiteralsTotal = 0;
	minimizationLiteralsRemoved = 0;
	minimizationTests = 0;

	hypothesisDegreeMin = std::numeric_limits<size_t>::max();
	hypothesisDegreeTotal = 0;
	hypothesisDegreeMax = 0;

	hypothesisLiteralsMin = std::numeric_limits<size_t>::max();
	hypothesisLiteralsTotal = 0;
	hypothesisLiteralsMax = 0;

	hypothesesTested = 0;
	hypothesesSkippedContainsTerminalLiteral = 0;
	hypothesesSkippedDegreeTooHigh = 0;
	hypothesesSkippedContainsTooManyLiterals = 0;
	hypothesesSkippedSubsumed = 0;

	feedbackExtractionTimeTotal = 0.0;
	feedbackExtractionConstraintsTotal = 0;
	feedbackExtractionResumes = 0;
	feedbackExtractionRestarts = 0;

	penalty = 0.0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void ProductionAnalysis<Plain>::write(const boost::filesystem::path &path) const
{
	setlocale(LC_NUMERIC, "C");

	Json::Value json;
	auto &jsonConfiguration = json["Configuration"];

	json["Runtime"] = runtime;
	json["FinishedNormally"] = finishedNormally;

	json["Proofs"] = static_cast<Json::UInt64>(proofs);
	json["ProofsSuccessful"] = static_cast<Json::UInt64>(proofsSuccessful);
	json["ProofTimeGroundingTotal"] = proofTimeGroundingTotal;
	json["ProofTimeSolvingTotal"] = proofTimeSolvingTotal;

	json["MinimizationProofs"] = static_cast<Json::UInt64>(minimizationProofs);
	json["MinimizationProofsSuccessful"] = static_cast<Json::UInt64>(minimizationProofsSuccessful);
	json["MinimizationProofTimeGroundingTotal"] = minimizationProofTimeGroundingTotal;
	json["MinimizationProofTimeSolvingTotal"] = minimizationProofTimeSolvingTotal;

	json["MinimizationTests"] = static_cast<Json::UInt64>(minimizationTests);
	json["MinimizationLiteralsTotal"] = static_cast<Json::UInt64>(minimizationLiteralsTotal);
	json["MinimizationLiteralsRemoved"] = static_cast<Json::UInt64>(minimizationLiteralsRemoved);

	json["HypothesisDegreeMin"] = static_cast<Json::UInt64>(hypothesisDegreeMin);
	json["HypothesisDegreeTotal"] = static_cast<Json::UInt64>(hypothesisDegreeTotal);
	json["HypothesisDegreeMax"] = static_cast<Json::UInt64>(hypothesisDegreeMax);

	json["HypothesisLiteralsMin"] = static_cast<Json::UInt64>(hypothesisLiteralsMin);
	json["HypothesisLiteralsTotal"] = static_cast<Json::UInt64>(hypothesisLiteralsTotal);
	json["HypothesisLiteralsMax"] = static_cast<Json::UInt64>(hypothesisLiteralsMax);

	json["HypothesesTested"] = static_cast<Json::UInt64>(hypothesesTested);
	json["HypothesesSkippedContainsTerminalLiteral"] = static_cast<Json::UInt64>(hypothesesSkippedContainsTerminalLiteral);
	json["HypothesesSkippedDegreeToHigh"] = static_cast<Json::UInt64>(hypothesesSkippedDegreeTooHigh);
	json["HypothesesSkippedContainsTooManyLiterals"] = static_cast<Json::UInt64>(hypothesesSkippedContainsTooManyLiterals);
	json["HypothesesSkippedSubsumed"] = static_cast<Json::UInt64>(hypothesesSkippedSubsumed);

	json["FeedbackExtractionTimeTotal"] = feedbackExtractionTimeTotal;
	json["FeedbackExtractionConstraintsTotal"] = static_cast<Json::UInt64>(feedbackExtractionConstraintsTotal);
	json["FeedbackExtractionResumes"] = static_cast<Json::UInt64>(feedbackExtractionResumes);
	json["FeedbackExtractionRestarts"] = static_cast<Json::UInt64>(feedbackExtractionRestarts);

	json["Penalty"] = penalty;

	jsonConfiguration = configuration.toJSON();

	std::fstream file(path.string(), std::ios::out);

	file << json;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

const boost::filesystem::path &ProductionAnalysis<Plain>::instance() const
{
	return configuration.instance;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

const boost::filesystem::path &ProductionAnalysis<Plain>::domain() const
{
	return configuration.domain;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

}
}
}
