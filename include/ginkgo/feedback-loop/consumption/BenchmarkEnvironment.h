#ifndef __FEEDBACK_LOOP__CONSUMPTION__BENCHMARK_ENVIRONMENT_H
#define __FEEDBACK_LOOP__CONSUMPTION__BENCHMARK_ENVIRONMENT_H

#include <memory>
#include <fstream>
#include <boost/filesystem.hpp>

#include <ginkgo/solving/Constraints.h>
#include <ginkgo/solving/AsyncProcess.h>

#include <ginkgo/feedback-loop/consumption/FeedbackType.h>

namespace ginkgo
{
namespace feedbackLoop
{
namespace consumption
{

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// BenchmarkEnvironment
//
////////////////////////////////////////////////////////////////////////////////////////////////////

class BenchmarkEnvironment
{
	public:
		BenchmarkEnvironment(boost::filesystem::path filePrefix);

		void setClaspConfiguration(AsyncProcess::Configuration claspConfiguration);
		const AsyncProcess::Configuration &claspConfiguration() const;

		void setGringoConfiguration(AsyncProcess::Configuration gringoConfiguration);
		const AsyncProcess::Configuration &gringoConfiguration() const;

		void setFeedbackType(FeedbackType feedbackType);
		FeedbackType feedbackType() const;

		void setHorizon(size_t horizon);
		size_t horizon() const;

		void setMaxNumberOfConstraints(size_t maxNumberOfConstraints);
		size_t maxNumberOfConstraints() const;

		std::ifstream &constraintsStream();
		std::ifstream &productionStatisticsStream();
		std::ofstream &consumptionStatisticsStream();

	private:
		boost::filesystem::path m_filePrefix;

		AsyncProcess::Configuration m_claspConfiguration;
		AsyncProcess::Configuration m_gringoConfiguration;

		FeedbackType m_feedbackType;
		size_t m_horizon;
		size_t m_maxNumberOfConstraints;

		std::ifstream m_constraintsStream;
		std::ifstream m_productionStatisticsStream;
		std::ofstream m_consumptionStatisticsStream;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

}
}
}

#endif
