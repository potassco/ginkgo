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
	// Total time spent grounding while testing candidates
	typename S<double>::Numerical proofTimeGroundingTotal;
	// Total time spent solving while testing candidates
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
	// Total number of proofs required to minimize candidates
	typename S<size_t>::Numerical minimizationTests;

	// Minimum degree of a tested candidate
	typename S<size_t>::Numerical candidateDegreeMin;
	// Sum of all degrees of tested candidates
	typename S<size_t>::Numerical candidateDegreeTotal;
	// Maximum degree of a tested candidate
	typename S<size_t>::Numerical candidateDegreeMax;

	// Minimum number of literals of a tested candidate
	typename S<size_t>::Numerical candidateLiteralsMin;
	// Total number of literals of tested candidates
	typename S<size_t>::Numerical candidateLiteralsTotal;
	// Maximum number of literals of a tested candidate
	typename S<size_t>::Numerical candidateLiteralsMax;

	// Total number of tested candidates (not counting minimization)
	typename S<size_t>::Numerical candidatesTested;
	// Total number of candidates skipped because of terminal literal
	typename S<size_t>::Numerical candidatesSkippedContainsTerminalLiteral;
	// Total number of candidates skipped because degree was too high
	typename S<size_t>::Numerical candidatesSkippedDegreeTooHigh;
	// Total number of candidates skipped because of too many literals
	typename S<size_t>::Numerical candidatesSkippedContainsTooManyLiterals;
	// Total number of candidates skipped because of subsumption
	typename S<size_t>::Numerical candidatesSkippedSubsumed;

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

	aggregatedAnalysis.candidateDegreeMin.aggregate(plainAnalyses, [&](const auto &e) {return accessor(e).candidateDegreeMin;}, selector);
	aggregatedAnalysis.candidateDegreeTotal.aggregate(plainAnalyses, [&](const auto &e) {return accessor(e).candidateDegreeTotal;}, selector);
	aggregatedAnalysis.candidateDegreeMax.aggregate(plainAnalyses, [&](const auto &e) {return accessor(e).candidateDegreeMax;}, selector);

	aggregatedAnalysis.candidateLiteralsMin.aggregate(plainAnalyses, [&](const auto &e) {return accessor(e).candidateLiteralsMin;}, selector);
	aggregatedAnalysis.candidateLiteralsTotal.aggregate(plainAnalyses, [&](const auto &e) {return accessor(e).candidateLiteralsTotal;}, selector);
	aggregatedAnalysis.candidateLiteralsMax.aggregate(plainAnalyses, [&](const auto &e) {return accessor(e).candidateLiteralsMax;}, selector);

	aggregatedAnalysis.candidatesTested.aggregate(plainAnalyses, [&](const auto &e) {return accessor(e).candidatesTested;}, selector);
	aggregatedAnalysis.candidatesSkippedContainsTerminalLiteral.aggregate(plainAnalyses, [&](const auto &e) {return accessor(e).candidatesSkippedContainsTerminalLiteral;}, selector);
	aggregatedAnalysis.candidatesSkippedDegreeTooHigh.aggregate(plainAnalyses, [&](const auto &e) {return accessor(e).candidatesSkippedDegreeTooHigh;}, selector);
	aggregatedAnalysis.candidatesSkippedContainsTooManyLiterals.aggregate(plainAnalyses, [&](const auto &e) {return accessor(e).candidatesSkippedContainsTooManyLiterals;}, selector);
	aggregatedAnalysis.candidatesSkippedSubsumed.aggregate(plainAnalyses, [&](const auto &e) {return accessor(e).candidatesSkippedSubsumed;}, selector);

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
