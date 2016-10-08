#ifndef __FEEDBACK_LOOP__PRODUCTION__ENVIRONMENT_H
#define __FEEDBACK_LOOP__PRODUCTION__ENVIRONMENT_H

#include <chrono>
#include <fstream>
#include <memory>

#include <boost/filesystem.hpp>

#include <ginkgo/feedback-loop/production/FluentClosureUsage.h>
#include <ginkgo/feedback-loop/production/LogLevel.h>
#include <ginkgo/feedback-loop/production/MinimizationStrategy.h>
#include <ginkgo/feedback-loop/production/ProofMethod.h>
#include <ginkgo/feedback-loop/production/TestingPolicy.h>

#include <ginkgo/solving/Constraints.h>
#include <ginkgo/solving/AsyncProcess.h>

namespace ginkgo
{
namespace feedbackLoop
{
namespace production
{

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Environment
//
////////////////////////////////////////////////////////////////////////////////////////////////////

class Environment
{
	public:
		Environment(boost::filesystem::path outputPrefix);

		std::ofstream &directConstraintsStream();
		std::ofstream &generalizedConstraintsStream();
		std::ofstream &statisticsStream();

		void setClaspConfiguration(AsyncProcess::Configuration claspConfiguration);
		const AsyncProcess::Configuration &claspConfiguration() const;

		void setXclaspConfiguration(AsyncProcess::Configuration XclaspConfiguration);
		const AsyncProcess::Configuration &xclaspConfiguration() const;

		void setGringoConfiguration(AsyncProcess::Configuration gringoConfiguration);
		const AsyncProcess::Configuration &gringoConfiguration() const;

		SymbolTable &symbolTable();
		const SymbolTable &symbolTable() const;

		void setLogLevel(LogLevel logLevel);
		LogLevel logLevel() const;

	private:
		boost::filesystem::path m_outputPrefix;

		std::ofstream m_directConstraintsStream;
		std::ofstream m_generalizedConstraintsStream;
		std::ofstream m_statisticsStream;

		AsyncProcess::Configuration m_claspConfiguration;
		AsyncProcess::Configuration m_xclaspConfiguration;
		AsyncProcess::Configuration m_gringoConfiguration;

		SymbolTable m_symbolTable;

		LogLevel m_logLevel;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

}
}
}

#endif
