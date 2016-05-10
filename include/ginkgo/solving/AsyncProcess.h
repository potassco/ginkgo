#ifndef __SOLVING__ASYNC_PROCESS_H
#define __SOLVING__ASYNC_PROCESS_H

#include <sstream>
#include <thread>
#include <array>
#include <mutex>
#include <deque>
#include <chrono>
#include <iostream>

#include <ginkgo/utils/Semaphore.h>

namespace ginkgo
{

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// AsyncProcess
//
////////////////////////////////////////////////////////////////////////////////////////////////////

class AsyncProcess
{
	public:
		struct Configuration
		{
			std::string binary;
			std::initializer_list<std::string> arguments;
		};

	public:
		AsyncProcess(Configuration configuration);

		std::mutex &stdoutAccessMutex();
		std::stringstream *stdout();
		void clearStdout();

		std::mutex &stderrAccessMutex();
		std::stringstream *stderr();
		void clearStderr();

		bool waitForEvent();
		bool waitForEvent(const std::chrono::milliseconds &relativeTime, std::cv_status &cvStatus);

		void run(std::stringstream &stdin, bool splitStdoutLines = false, bool splitStderrLines = false);
		void run(std::stringstream &stdin, const std::chrono::milliseconds &relativeTime, bool &timeout, bool splitStdoutLines = false, bool splitStderrLines = false);
		bool isRunning() const;

		int exitCode() const;

		void pause() const;
		void resume() const;
		void terminate() const;
		void kill() const;
		bool exists() const;

		void join() const;

	private:
		struct Pipe
		{
			std::deque<std::stringstream> streams;
			bool splitLines;
			std::mutex accessMutex;
		};

	private:
		void runThread(std::stringstream &stdin);
		void runChildProcess();
		void runParentProcess(std::stringstream &stdin);

		Configuration m_configuration;

		std::array<int, 2> m_inPipe;
		std::array<int, 2> m_outPipe;
		std::array<int, 2> m_errPipe;

		Pipe m_stdout;
		Pipe m_stderr;

		Semaphore m_eventSemaphore;

		pid_t m_childPID;

		int m_exitCode;

		mutable std::unique_ptr<std::thread> m_thread;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

}

#endif
