#ifndef __FEEDBACK_LOOP__PRODUCTION__ENVIRONMENT_H
#define __FEEDBACK_LOOP__PRODUCTION__ENVIRONMENT_H

#include <chrono>
#include <fstream>
#include <memory>

#include <boost/filesystem.hpp>

#include <ginkgo/feedback-loop/production/LogLevel.h>

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

		void setLogLevel(LogLevel logLevel);
		LogLevel logLevel() const;

	private:
		boost::filesystem::path m_outputPrefix;

		std::ofstream m_directConstraintsStream;
		std::ofstream m_generalizedConstraintsStream;
		std::ofstream m_statisticsStream;

		LogLevel m_logLevel;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

}
}
}

#endif
