#ifndef __FEEDBACK_LOOP__ANALYSIS__ANALYSIS_H
#define __FEEDBACK_LOOP__ANALYSIS__ANALYSIS_H

#include <functional>

#include <boost/filesystem.hpp>

#include <ginkgo/feedback-loop/analysis/ProductionAnalysis.h>
#include <ginkgo/feedback-loop/analysis/ConsumptionAnalysis.h>
#include <ginkgo/feedback-loop/consumption/FeedbackType.h>
#include <ginkgo/utils/Statistics.h>

namespace ginkgo
{
namespace feedbackLoop
{
namespace analysis
{

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Analysis
//
////////////////////////////////////////////////////////////////////////////////////////////////////

template <template<class> class S>
struct AnalysisBase
{
	ProductionAnalysis<S> productionAnalysis;
	std::map<consumption::FeedbackType, ConsumptionAnalysis<S>> consumptionAnalyses;

	std::vector<consumption::FeedbackType> feedbackTypes() const
	{
		std::vector<consumption::FeedbackType> feedbackTypes;

		std::for_each(consumptionAnalyses.cbegin(), consumptionAnalyses.cend(),
			[&](const auto &consumptionAnalysis)
			{
				feedbackTypes.push_back(std::get<0>(consumptionAnalysis));
			});

		return feedbackTypes;
	}
};

////////////////////////////////////////////////////////////////////////////////////////////////////

template <template<class> class S>
struct Analysis : public AnalysisBase<S>
{
};

////////////////////////////////////////////////////////////////////////////////////////////////////

template <>
struct Analysis<Plain> : public AnalysisBase<Plain>
{
	static Analysis fromFiles(const boost::filesystem::path &filePrefix);
};

////////////////////////////////////////////////////////////////////////////////////////////////////

template <>
struct Analysis<Aggregated> : public AnalysisBase<Aggregated>
{
	static Analysis aggregate(const std::vector<Analysis<Plain>> &plainAnalyses,
		std::function<bool (const Analysis<Plain> &)> selector = [](const auto &){return true;});
};

////////////////////////////////////////////////////////////////////////////////////////////////////

}
}
}

#endif
