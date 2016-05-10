#include <ginkgo/feedback-loop/production/LogLevel.h>

#include <iostream>
#include <boost/bimap.hpp>
#include <boost/assign.hpp>

namespace ginkgo
{
namespace feedbackLoop
{
namespace production
{

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// LogLevel
//
////////////////////////////////////////////////////////////////////////////////////////////////////

using LogLevelNames = boost::bimap<LogLevel, std::string>;
static LogLevelNames logLevelNames = boost::assign::list_of<LogLevelNames::relation>
	(LogLevel::Normal, "Normal")
	(LogLevel::Debug, "Debug");

////////////////////////////////////////////////////////////////////////////////////////////////////

std::ostream &operator<<(std::ostream &ostream, const LogLevel &logLevel)
{
	const auto match = logLevelNames.left.find(logLevel);

	if (match == logLevelNames.left.end())
		return (ostream << "Unknown");

	return (ostream << (*match).second);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::istream &operator>>(std::istream &istream, LogLevel &logLevel)
{
	std::string logLevelName;
	istream >> logLevelName;

	const auto match = logLevelNames.right.find(logLevelName);

	if (match == logLevelNames.right.end())
		logLevel = LogLevel::Unknown;
	else
		logLevel = (*match).second;

	return istream;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

}
}
}
