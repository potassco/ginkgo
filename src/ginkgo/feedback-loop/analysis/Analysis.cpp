#include <ginkgo/feedback-loop/analysis/Analysis.h>

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

Analysis<Plain> Analysis<Plain>::fromFiles(const boost::filesystem::path &filePrefix)
{
	const auto productionAnalysisFile = filePrefix.string() + ".analysis-produce";
	const auto consumptionDirectAnalysisFile = filePrefix.string() + ".analysis-consume-direct";
	const auto consumptionGeneralizedAnalysisFile = filePrefix.string() + ".analysis-consume-generalized";

	const auto checkFile =
		[](const auto &path)
		{
			if (boost::filesystem::is_regular_file(path))
				return true;

			return false;
		};

	Analysis<Plain> analysis;

	if (checkFile(productionAnalysisFile))
		analysis.productionAnalysis = ProductionAnalysis<Plain>::fromAnalysisFile(productionAnalysisFile);
	else
		std::cerr << "[Error] File not found: " << productionAnalysisFile << std::endl;

	if (checkFile(consumptionDirectAnalysisFile))
		analysis.consumptionAnalyses[consumption::FeedbackType::Direct] = ConsumptionAnalysis<Plain>::fromAnalysisFile(consumptionDirectAnalysisFile);
	else
		std::cerr << "[Error] File not found: " << consumptionDirectAnalysisFile << std::endl;

	if (checkFile(consumptionGeneralizedAnalysisFile))
		analysis.consumptionAnalyses[consumption::FeedbackType::Generalized] = ConsumptionAnalysis<Plain>::fromAnalysisFile(consumptionGeneralizedAnalysisFile);
	else
		std::cerr << "[Error] File not found: " << consumptionGeneralizedAnalysisFile << std::endl;

	return analysis;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Analysis<Aggregated> Analysis<Aggregated>::aggregate(
	const std::vector<Analysis<Plain>> &plainAnalyses,
	std::function<bool (const Analysis<Plain> &)> selector)
{
	Analysis<Aggregated> aggregatedAnalysis;

	const auto productionAnalysisAccessor = [](const auto &e) {return e.productionAnalysis;};
	aggregatedAnalysis.productionAnalysis = ProductionAnalysis<Aggregated>::aggregate(plainAnalyses, productionAnalysisAccessor, selector);

	const auto feedbackTypes = plainAnalyses.front().feedbackTypes();

	std::for_each(plainAnalyses.cbegin(), plainAnalyses.cend(),
		[&](const auto &plainAnalysis)
		{
			const auto otherFeedbackTypes = plainAnalysis.feedbackTypes();

			BOOST_ASSERT_MSG(feedbackTypes == otherFeedbackTypes,
				"Inconsistent feedback conditions, cannot aggregate");
		});

	std::for_each(feedbackTypes.cbegin(), feedbackTypes.cend(),
		[&](const auto &feedbackType)
		{
			const auto consumptionAnalysisAccessor =
				[&](const auto &e)
				{
					const auto match = e.consumptionAnalyses.find(feedbackType);
					BOOST_ASSERT(match != e.consumptionAnalyses.cend());
					return std::get<1>(*match);
				};
			aggregatedAnalysis.consumptionAnalyses[feedbackType] = ConsumptionAnalysis<Aggregated>::aggregate(plainAnalyses, consumptionAnalysisAccessor, selector);
		});

	return aggregatedAnalysis;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

}
}
}
