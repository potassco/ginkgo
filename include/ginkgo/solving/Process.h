#ifndef __SOLVING__PROCESS_H
#define __SOLVING__PROCESS_H

#include <string>
#include <sstream>
#include <mutex>
#include <condition_variable>
#include <vector>

namespace ginkgo
{

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Process
//
////////////////////////////////////////////////////////////////////////////////////////////////////

class Process
{
	public:
		Process(std::string binary, const std::vector<std::string> &arguments);
		virtual ~Process();

		virtual void run(std::istream &input, bool reportErrors = true);
		virtual void run(std::istream &input, const std::vector<std::string> &additionalArguments, bool reportErrors = true);

		std::stringstream &stdout();
		std::mutex &stdoutMutex();
		std::stringstream &stderr();
		std::mutex &stderrMutex();

		bool isStdoutEmpty() const;
		bool isStderrEmpty() const;

		int exitCode() const;

	protected:
		void runChildProcess();
		void runParentProcess(std::istream &input, bool reportErrors = true);

		std::string m_binary;
		std::vector<char *> m_arguments;

		int m_inPipe[2];
		int m_outPipe[2];
		int m_errPipe[2];

		pid_t m_childPID;

		std::stringstream m_stdout;
		std::mutex m_stdoutMutex;
		std::stringstream m_stderr;
		std::mutex m_stderrMutex;

		int m_exitCode;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

}

#endif
