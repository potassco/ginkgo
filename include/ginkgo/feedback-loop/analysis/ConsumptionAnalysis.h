#ifndef __FEEDBACK_LOOP__ANALYSIS__CONSUMPTION_ANALYSIS_H
#define __FEEDBACK_LOOP__ANALYSIS__CONSUMPTION_ANALYSIS_H

#include <boost/filesystem.hpp>

#include <json/value.h>

#include <ginkgo/feedback-loop/production/Events.h>
#include <ginkgo/feedback-loop/consumption/FeedbackType.h>

namespace ginkgo
{
namespace feedbackLoop
{
namespace analysis
{

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// ConsumptionAnalysis
//
////////////////////////////////////////////////////////////////////////////////////////////////////

template <template<class> class S>
struct ConsumptionAnalysisBase
{
	struct TimeMeasurement
	{
		typename S<bool>::Numerical timeout;
		typename S<double>::Numerical time;
	};

	struct Measurement
	{
		size_t selectedConstraints;
		TimeMeasurement timeMeasurement;
	};

	void sort()
	{
		std::sort(measurements.begin(), measurements.end(),
			[&](const auto &a, const auto &b)
			{
				return a.selectedConstraints < b.selectedConstraints;
			});
	}

	std::vector<Measurement> measurements;

	typename S<double>::Set penalty;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

template <template<class> class S>
struct ConsumptionAnalysis : public ConsumptionAnalysisBase<S>
{
};

////////////////////////////////////////////////////////////////////////////////////////////////////

template <>
struct ConsumptionAnalysis<Plain> : public ConsumptionAnalysisBase<Plain>
{
	static ConsumptionAnalysis fromRawFile(const boost::filesystem::path &path, double penalty);
	static ConsumptionAnalysis fromAnalysisFile(const boost::filesystem::path &path);
	void write(const boost::filesystem::path &path) const;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

template <>
struct ConsumptionAnalysis<Aggregated> : public ConsumptionAnalysisBase<Aggregated>
{
	template <class Container, class Accessor, class Selector>
	static ConsumptionAnalysis aggregate(const Container &plainAnalyses,
		Accessor accessor, Selector selector = [](const auto &){return true;});
};

////////////////////////////////////////////////////////////////////////////////////////////////////

template <class Container, class Accessor, class Selector>
ConsumptionAnalysis<Aggregated> ConsumptionAnalysis<Aggregated>::aggregate(
	const Container &plainAnalyses, const Accessor accessor, const Selector selector)
{
	ConsumptionAnalysis<Aggregated> aggregatedAnalysis;

	aggregatedAnalysis.penalty.aggregate(plainAnalyses, [&](const auto &e) {return accessor(e).penalty;}, selector);

	size_t numberOfMeasurements = accessor(plainAnalyses.front()).measurements.size();

	std::for_each(plainAnalyses.cbegin(), plainAnalyses.cend(),
		[&](const auto &plainAnalysis)
		{
			BOOST_ASSERT_MSG(accessor(plainAnalysis).measurements.size() == numberOfMeasurements,
				"Inconsistent numbers of measurements, cannot aggregate");
		});

	for (size_t i = 0; i < numberOfMeasurements; i++)
	{
		Measurement measurement;
		measurement.selectedConstraints = accessor(plainAnalyses.front()).measurements[i].selectedConstraints;

		std::for_each(plainAnalyses.cbegin(), plainAnalyses.cend(),
			[&](const auto &plainAnalysis)
			{
				BOOST_ASSERT_MSG(accessor(plainAnalysis).measurements[i].selectedConstraints == measurement.selectedConstraints,
					"Inconsistent numbers of selected constraints, cannot aggregate");
			});

		measurement.timeMeasurement.timeout.aggregate(plainAnalyses, [&](const auto &e) {return accessor(e).measurements[i].timeMeasurement.timeout;}, selector);
		measurement.timeMeasurement.time.aggregate(plainAnalyses, [&](const auto &e) {return accessor(e).measurements[i].timeMeasurement.time;}, selector);

		aggregatedAnalysis.measurements.emplace_back(measurement);
	}

	return aggregatedAnalysis;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

}
}
}

#endif
