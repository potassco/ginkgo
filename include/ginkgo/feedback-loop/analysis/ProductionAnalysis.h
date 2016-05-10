#ifndef __FEEDBACK_LOOP__ANALYSIS__PRODUCTION_ANALYSIS_H
#define __FEEDBACK_LOOP__ANALYSIS__PRODUCTION_ANALYSIS_H

#include <boost/filesystem.hpp>

#include <ginkgo/feedback-loop/production/Events.h>
#include <ginkgo/feedback-loop/production/Configuration.h>
#include <ginkgo/utils/Statistics.h>

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

template <template<class> class S>
struct ProductionAnalysisBase
{
	// Total runtime of the feedback loop
	typename S<double>::Numerical runtime;
	// Whether the feedback loop terminated normally or not
	typename S<bool>::Numerical finishedNormally;

	// Total number of performed proofs (without minimization)
	typename S<size_t>::Numerical proofs;
	// Number of successful proofs
	typename S<size_t>::Numerical proofsSuccessful;
	// Total time spent grounding while testing hypotheses
	typename S<double>::Numerical proofTimeGroundingTotal;
	// Total time spent solving while testing hypotheses
	typename S<double>::Numerical proofTimeSolvingTotal;

	// Total number of performed minimization proofs
	typename S<size_t>::Numerical minimizationProofs;
	// Number of successful minimization proofs
	typename S<size_t>::Numerical minimizationProofsSuccessful;
	// Total time spent grounding while minimizing constraints
	typename S<double>::Numerical minimizationProofTimeGroundingTotal;
	// Total time spent solving while minimizing constraints
	typename S<double>::Numerical minimizationProofTimeSolvingTotal;

	// Total number of literals considered for minimization
	typename S<size_t>::Numerical minimizationLiteralsTotal;
	// Total number of literals removed by minimization
	typename S<size_t>::Numerical minimizationLiteralsRemoved;
	// Total number of proofs required to minimize hypotheses
	typename S<size_t>::Numerical minimizationTests;

	// Minimum degree of a tested hypothesis
	typename S<size_t>::Numerical hypothesisDegreeMin;
	// Sum of all degrees of tested hypotheses
	typename S<size_t>::Numerical hypothesisDegreeTotal;
	// Maximum degree of a tested hypothesis
	typename S<size_t>::Numerical hypothesisDegreeMax;

	// Minimum number of literals of a tested hypothesis
	typename S<size_t>::Numerical hypothesisLiteralsMin;
	// Total number of literals of tested hypotheses
	typename S<size_t>::Numerical hypothesisLiteralsTotal;
	// Maximum number of literals of a tested hypothesis
	typename S<size_t>::Numerical hypothesisLiteralsMax;

	// Total number of tested hypotheses (not counting minimization)
	typename S<size_t>::Numerical hypothesesTested;
	// Total number of hypotheses skipped because of terminal literal
	typename S<size_t>::Numerical hypothesesSkippedContainsTerminalLiteral;
	// Total number of hypotheses skipped because degree was too high
	typename S<size_t>::Numerical hypothesesSkippedDegreeTooHigh;
	// Total number of hypotheses skipped because of too many literals
	typename S<size_t>::Numerical hypothesesSkippedContainsTooManyLiterals;
	// Total number of hypotheses skipped because of subsumption
	typename S<size_t>::Numerical hypothesesSkippedSubsumed;

	// Total time spent extracting knowledge
	typename S<double>::Numerical feedbackExtractionTimeTotal;
	// Total number of conflict constraints extracted
	typename S<size_t>::Numerical feedbackExtractionConstraintsTotal;
	// How many times the knowledge extraction procedure was resumed
	typename S<size_t>::Numerical feedbackExtractionResumes;
	// How many times the knowledge extraction procedure was restarted
	typename S<size_t>::Numerical feedbackExtractionRestarts;

	// Penalty for timeouts
	typename S<double>::Numerical penalty;

	// Configuration used for feedback production
	production::Configuration<S> configuration;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

template <template<class> class S>
struct ProductionAnalysis : public ProductionAnalysisBase<S>
{
};

////////////////////////////////////////////////////////////////////////////////////////////////////

template <>
struct ProductionAnalysis<Plain> : public ProductionAnalysisBase<Plain>
{
	static ProductionAnalysis fromRawFile(const boost::filesystem::path &path, double penalty);
	static ProductionAnalysis fromAnalysisFile(const boost::filesystem::path &path);

	ProductionAnalysis<Plain>();

	void write(const boost::filesystem::path &path) const;

	const boost::filesystem::path &instance() const;
	const boost::filesystem::path &domain() const;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

template <>
struct ProductionAnalysis<Aggregated> : public ProductionAnalysisBase<Aggregated>
{
	template <class Container, class Accessor, class Selector>
	static ProductionAnalysis aggregate(const Container &plainAnalyses,
		Accessor accessor, Selector selector = [](const auto &){return true;});
};

////////////////////////////////////////////////////////////////////////////////////////////////////

template <class Container, class Accessor, class Selector>
ProductionAnalysis<Aggregated> ProductionAnalysis<Aggregated>::aggregate(
	const Container &plainAnalyses, Accessor accessor, Selector selector)
{
	ProductionAnalysis<Aggregated> aggregatedAnalysis;

	aggregatedAnalysis.runtime.aggregate(plainAnalyses, [&](const auto &e) {return accessor(e).runtime;}, selector);
	aggregatedAnalysis.finishedNormally.aggregate(plainAnalyses, [&](const auto &e) {return accessor(e).finishedNormally;}, selector);

	aggregatedAnalysis.proofs.aggregate(plainAnalyses, [&](const auto &e) {return accessor(e).proofs;}, selector);
	aggregatedAnalysis.proofsSuccessful.aggregate(plainAnalyses, [&](const auto &e) {return accessor(e).proofsSuccessful;}, selector);
	aggregatedAnalysis.proofTimeGroundingTotal.aggregate(plainAnalyses, [&](const auto &e) {return accessor(e).proofTimeGroundingTotal;}, selector);
	aggregatedAnalysis.proofTimeSolvingTotal.aggregate(plainAnalyses, [&](const auto &e) {return accessor(e).proofTimeSolvingTotal;}, selector);

	aggregatedAnalysis.minimizationProofs.aggregate(plainAnalyses, [&](const auto &e) {return accessor(e).minimizationProofs;}, selector);
	aggregatedAnalysis.minimizationProofsSuccessful.aggregate(plainAnalyses, [&](const auto &e) {return accessor(e).minimizationProofsSuccessful;}, selector);
	aggregatedAnalysis.minimizationProofTimeGroundingTotal.aggregate(plainAnalyses, [&](const auto &e) {return accessor(e).minimizationProofTimeGroundingTotal;}, selector);
	aggregatedAnalysis.minimizationProofTimeSolvingTotal.aggregate(plainAnalyses, [&](const auto &e) {return accessor(e).minimizationProofTimeSolvingTotal;}, selector);

	aggregatedAnalysis.minimizationLiteralsTotal.aggregate(plainAnalyses, [&](const auto &e) {return accessor(e).minimizationLiteralsTotal;}, selector);
	aggregatedAnalysis.minimizationLiteralsRemoved.aggregate(plainAnalyses, [&](const auto &e) {return accessor(e).minimizationLiteralsRemoved;}, selector);

	aggregatedAnalysis.hypothesisDegreeMin.aggregate(plainAnalyses, [&](const auto &e) {return accessor(e).hypothesisDegreeMin;}, selector);
	aggregatedAnalysis.hypothesisDegreeTotal.aggregate(plainAnalyses, [&](const auto &e) {return accessor(e).hypothesisDegreeTotal;}, selector);
	aggregatedAnalysis.hypothesisDegreeMax.aggregate(plainAnalyses, [&](const auto &e) {return accessor(e).hypothesisDegreeMax;}, selector);

	aggregatedAnalysis.hypothesisLiteralsMin.aggregate(plainAnalyses, [&](const auto &e) {return accessor(e).hypothesisLiteralsMin;}, selector);
	aggregatedAnalysis.hypothesisLiteralsTotal.aggregate(plainAnalyses, [&](const auto &e) {return accessor(e).hypothesisLiteralsTotal;}, selector);
	aggregatedAnalysis.hypothesisLiteralsMax.aggregate(plainAnalyses, [&](const auto &e) {return accessor(e).hypothesisLiteralsMax;}, selector);

	aggregatedAnalysis.hypothesesTested.aggregate(plainAnalyses, [&](const auto &e) {return accessor(e).hypothesesTested;}, selector);
	aggregatedAnalysis.hypothesesSkippedContainsTerminalLiteral.aggregate(plainAnalyses, [&](const auto &e) {return accessor(e).hypothesesSkippedContainsTerminalLiteral;}, selector);
	aggregatedAnalysis.hypothesesSkippedDegreeTooHigh.aggregate(plainAnalyses, [&](const auto &e) {return accessor(e).hypothesesSkippedDegreeTooHigh;}, selector);
	aggregatedAnalysis.hypothesesSkippedContainsTooManyLiterals.aggregate(plainAnalyses, [&](const auto &e) {return accessor(e).hypothesesSkippedContainsTooManyLiterals;}, selector);
	aggregatedAnalysis.hypothesesSkippedSubsumed.aggregate(plainAnalyses, [&](const auto &e) {return accessor(e).hypothesesSkippedSubsumed;}, selector);

	aggregatedAnalysis.feedbackExtractionTimeTotal.aggregate(plainAnalyses, [&](const auto &e) {return accessor(e).feedbackExtractionTimeTotal;}, selector);
	aggregatedAnalysis.feedbackExtractionConstraintsTotal.aggregate(plainAnalyses, [&](const auto &e) {return accessor(e).feedbackExtractionConstraintsTotal;}, selector);
	aggregatedAnalysis.feedbackExtractionResumes.aggregate(plainAnalyses, [&](const auto &e) {return accessor(e).feedbackExtractionResumes;}, selector);
	aggregatedAnalysis.feedbackExtractionRestarts.aggregate(plainAnalyses, [&](const auto &e) {return accessor(e).feedbackExtractionRestarts;}, selector);

	aggregatedAnalysis.minimizationTests.aggregate(plainAnalyses, [&](const auto &e) {return accessor(e).minimizationTests;}, selector);

	aggregatedAnalysis.configuration = production::Configuration<Aggregated>::aggregate(plainAnalyses, [&](const auto &e) {return accessor(e).configuration;}, selector);

	return aggregatedAnalysis;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

}
}
}

#endif
