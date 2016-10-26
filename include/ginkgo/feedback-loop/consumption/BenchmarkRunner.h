#ifndef __FEEDBACK_LOOP__CONSUMPTION__BENCHMARK_RUNNER_H
#define __FEEDBACK_LOOP__CONSUMPTION__BENCHMARK_RUNNER_H

#include <memory>
#include <sstream>

#include <ginkgo/feedback-loop/production/Events.h>
#include <ginkgo/feedback-loop/consumption/BenchmarkEnvironment.h>

namespace ginkgo
{
namespace feedbackLoop
{
namespace consumption
{

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// BenchmarkRunner
//
////////////////////////////////////////////////////////////////////////////////////////////////////

class BenchmarkRunner
{
	public:
		BenchmarkRunner(std::unique_ptr<BenchmarkEnvironment> environment);

		void run();

	private:
		std::vector<std::string> m_constraints;
		std::stringstream m_program;

		std::unique_ptr<BenchmarkEnvironment> m_environment;

		production::Configuration<Plain> m_productionConfiguration;
		production::Events m_productionEvents;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

}
}
}

#endif
