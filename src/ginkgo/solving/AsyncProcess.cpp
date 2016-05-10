#include <ginkgo/solving/AsyncProcess.h>

#include <cstdio>
#include <unistd.h>
#include <fcntl.h>

#include <iostream>
#include <string>
#include <thread>
#include <algorithm>
#include <cstring>
#include <cerrno>
#include <sys/wait.h>
#include <sys/epoll.h>

#include <boost/filesystem.hpp>
#include <boost/assert.hpp>

namespace ginkgo
{

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// AsyncProcess
//
////////////////////////////////////////////////////////////////////////////////////////////////////

AsyncProcess::AsyncProcess(Configuration configuration)
:	m_configuration(configuration),
	m_childPID{0},
	m_exitCode{-1}
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::mutex &AsyncProcess::stdoutAccessMutex()
{
	return m_stdout.accessMutex;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::mutex &AsyncProcess::stderrAccessMutex()
{
	return m_stderr.accessMutex;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::stringstream *AsyncProcess::stdout()
{
	if (m_stdout.streams.size() <= 1)
		return nullptr;

	// Get size of line
	auto &frontStream = m_stdout.streams.front();
	frontStream.seekg(0, std::ios::end);
	const auto frontStreamSize = frontStream.tellg();
	frontStream.seekg(0, std::ios::beg);

	if (frontStreamSize <= 0)
		return nullptr;

	return &frontStream;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::stringstream *AsyncProcess::stderr()
{
	if (m_stderr.streams.size() <= 1)
		return nullptr;

	// Get size of line
	auto &frontStream = m_stderr.streams.front();
	frontStream.seekg(0, std::ios::end);
	const auto frontStreamSize = frontStream.tellg();
	frontStream.seekg(0, std::ios::beg);

	if (frontStreamSize <= 0)
		return nullptr;

	return &frontStream;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void AsyncProcess::clearStdout()
{
	m_stdout.streams.pop_front();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void AsyncProcess::clearStderr()
{
	m_stderr.streams.pop_front();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool AsyncProcess::waitForEvent()
{
	m_eventSemaphore.wait();

	return m_eventSemaphore.count() > 0 || m_eventSemaphore.isActive();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool AsyncProcess::waitForEvent(const std::chrono::milliseconds &relativeTime, std::cv_status &cvStatus)
{
	cvStatus = m_eventSemaphore.waitFor(relativeTime);

	return m_eventSemaphore.count() > 0 || m_eventSemaphore.isActive();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void AsyncProcess::run(std::stringstream &stdin, bool splitStdoutLines, bool splitStderrLines)
{
	m_childPID = 0;
	m_exitCode = -1;
	m_eventSemaphore.reset();

	m_stdout.splitLines = splitStdoutLines;
	m_stderr.splitLines = splitStderrLines;

	m_thread = std::make_unique<std::thread>([this, &stdin]()
	{
		runThread(stdin);
	});
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void AsyncProcess::run(std::stringstream &stdin, const std::chrono::milliseconds &relativeTime,
	bool &timeout, bool splitStdoutLines, bool splitStderrLines)
{
	timeout = false;

	run(stdin, splitStdoutLines, splitStderrLines);

	if (relativeTime == std::chrono::milliseconds(0))
		return;

	std::cv_status cvStatus;

	while (waitForEvent(relativeTime, cvStatus))
	{
		if (cvStatus != std::cv_status::timeout)
			continue;

		// Handle timeouts
		kill();
		timeout = true;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool AsyncProcess::isRunning() const
{
	return m_eventSemaphore.isActive();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void AsyncProcess::runThread(std::stringstream &stdin)
{
	m_stdout.streams.clear();
	m_stdout.streams.resize(1);
	m_stderr.streams.clear();
	m_stderr.streams.resize(1);

	// Pipe for stdin
	if (pipe(m_inPipe.data()) == -1 || pipe(m_outPipe.data()) == -1 || pipe(m_errPipe.data()) == -1)
	{
		std::cerr << "[Error] Could not create pipes" << std::endl;
		return;
	}

	const pid_t pid = fork();

	if (pid == -1)
	{
		std::cerr << "[Error] Could not fork process" << std::endl;
		return;
	}
	// Child process
	else if (pid == 0)
		runChildProcess();
	// Parent process
	else
	{
		m_childPID = pid;
		runParentProcess(stdin);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void AsyncProcess::runParentProcess(std::stringstream &stdin)
{
	const auto &childReadIn = m_inPipe[0];
	const auto &parentWriteIn = m_inPipe[1];
	const auto &parentReadOut = m_outPipe[0];
	const auto &childWriteOut = m_outPipe[1];
	const auto &parentReadErr = m_errPipe[0];
	const auto &childWriteErr = m_errPipe[1];

	close(childReadIn);
	close(childWriteOut);
	close(childWriteErr);

	std::array<std::stringstream::char_type, 1024> buffer;

	const auto maxFD = std::max(std::max(parentReadOut, parentReadErr), parentWriteIn);

	const auto handleWrite = [&]()
	{
		if (!stdin.good())
		{
			close(parentWriteIn);
			stdin.str(std::string());
			return false;
		}

		stdin.read(buffer.data(), buffer.size());
		const auto count = stdin.gcount();

		const auto written = write(parentWriteIn, buffer.data(), count);

		if (written != count)
			std::cerr << "[Error] Couldn't write entire buffer" << std::endl;

		return true;
	};

	const auto handleRead = [&](auto fileDescriptor, auto &outputPipe)
	{
		const auto count = read(fileDescriptor, buffer.data(), buffer.size());

		if (count == 0)
		{
			close(fileDescriptor);
			return false;
		}

		if (outputPipe.splitLines)
		{
			auto start = buffer.cbegin();
			auto end = buffer.cbegin() + count;

			while (auto newlinePosition = std::find(start, end, '\n'))
			{
				if (newlinePosition == end)
					break;

				{
					std::lock_guard<std::mutex> lock(outputPipe.accessMutex);

					auto &workingStream = outputPipe.streams.back();
					workingStream.write(start, newlinePosition - start).flush();
					workingStream << std::endl;

					outputPipe.streams.resize(outputPipe.streams.size() + 1);
				}

				m_eventSemaphore.notify();

				start = newlinePosition + 1;
			}

			if (start < end)
			{
				std::lock_guard<std::mutex> lock(outputPipe.accessMutex);
				outputPipe.streams.back().write(start, end - start).flush();
			}
		}
		else
		{
			std::lock_guard<std::mutex> lock(outputPipe.accessMutex);
			outputPipe.streams.back().write(buffer.data(), count);
		}

		return true;
	};

	bool parentReadOutActive = true;
	bool parentReadErrActive = true;
	bool parentWriteInActive = true;

	const auto waitForInput = [&]()
	{
		fd_set readFDSet;
		fd_set writeFDSet;

		int numberOfEvents = -1;

		while (true)
		{
			FD_ZERO(&readFDSet);
			FD_ZERO(&writeFDSet);

			if (parentReadOutActive)
				FD_SET(parentReadOut, &readFDSet);

			if (parentReadErrActive)
				FD_SET(parentReadErr, &readFDSet);

			if (parentWriteInActive)
				FD_SET(parentWriteIn, &writeFDSet);

			numberOfEvents = select(maxFD + 1, &readFDSet, &writeFDSet, nullptr, nullptr);

			if (numberOfEvents != -1 || errno != EINTR)
				break;
		}

		if (numberOfEvents < 0)
		{
			std::cerr << "[Error] select failed" << std::endl;
			exit(EXIT_FAILURE);
		}

		if (FD_ISSET(parentReadOut, &readFDSet))
			parentReadOutActive &= handleRead(parentReadOut, m_stdout);

		if (FD_ISSET(parentReadErr, &readFDSet))
			parentReadErrActive &= handleRead(parentReadErr, m_stderr);

		if (FD_ISSET(parentWriteIn, &writeFDSet))
			parentWriteInActive &= handleWrite();

		return parentReadOutActive || parentReadErrActive || parentWriteInActive;
	};

	while (waitForInput());

	const auto finishReading = [&](auto fileDescriptor, auto &outputPipe)
	{
		if (fcntl(fileDescriptor, F_GETFD) == -1 && errno == EBADF)
			return;

		while (handleRead(fileDescriptor, outputPipe));
	};

	finishReading(parentReadOut, m_stdout);
	finishReading(parentReadErr, m_stderr);

	close(parentReadOut);
	close(parentReadErr);

	// Clear fail flags from stdin
	stdin.clear();

	int status = 0;
	waitpid(m_childPID, &status, 0);

	m_exitCode = WEXITSTATUS(status);

	m_childPID = 0;

	if (m_exitCode == 255)
		std::cerr << "[Error] Could not execute process" << std::endl;

	// Add an empty stringstream to make readers see the last line of output
	{
		std::lock_guard<std::mutex> lock(m_stdout.accessMutex);
		m_stdout.streams.resize(m_stdout.streams.size() + 1);
	}
	{
		std::lock_guard<std::mutex> lock(m_stderr.accessMutex);
		m_stderr.streams.resize(m_stderr.streams.size() + 1);
	}

	// Notify about possibly pending lines
	m_eventSemaphore.notifyFinished();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void AsyncProcess::runChildProcess()
{
	const auto &childReadIn = m_inPipe[0];
	const auto &parentWriteIn = m_inPipe[1];
	const auto &parentReadOut = m_outPipe[0];
	const auto &childWriteOut = m_outPipe[1];
	const auto &parentReadErr = m_errPipe[0];
	const auto &childWriteErr = m_errPipe[1];

	close(parentWriteIn);
	close(parentReadOut);
	close(parentReadErr);

	if (dup2(childReadIn, STDIN_FILENO) < 0
		|| dup2(childWriteOut, STDOUT_FILENO) < 0
		|| dup2(childWriteErr, STDERR_FILENO) < 0)
	{
		perror("dup2 failed");
		exit(EXIT_FAILURE);
	}

	close(childReadIn);
	close(childWriteOut);
	close(childWriteErr);

	// Remove const qualifier, as execvp does not write to argv
	// http://pubs.opengroup.org/onlinepubs/9699919799/functions/exec.html
	const auto unconst = [](const auto &string)
	{
		return const_cast<char *>(string.c_str());
	};

	std::vector<char *> rawArguments;
	rawArguments.reserve(m_configuration.arguments.size() + 2);

	rawArguments.push_back(unconst(m_configuration.binary));
	std::transform(m_configuration.arguments.begin(), m_configuration.arguments.end(), std::back_inserter(rawArguments), unconst);
	rawArguments.push_back(nullptr);

	_exit(execvp(rawArguments[0], rawArguments.data()));
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool AsyncProcess::exists() const
{
	if (m_childPID == 0)
		return false;

	return ::kill(m_childPID, 0) == 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void AsyncProcess::terminate() const
{
	if (!exists())
	{
		std::cerr << "[Warn ] Process cannot be terminated (does not exist)" << std::endl;
		return;
	}

	::kill(m_childPID, SIGTERM);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void AsyncProcess::kill() const
{
	if (!exists())
	{
		std::cerr << "[Warn ] Process cannot be killed (does not exist)" << std::endl;
		return;
	}

	::kill(m_childPID, SIGKILL);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void AsyncProcess::pause() const
{
	if (!exists())
	{
		std::cerr << "[Warn ] Process cannot be paused (does not exist)" << std::endl;
		return;
	}

	::kill(m_childPID, SIGSTOP);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void AsyncProcess::resume() const
{
	if (!exists())
	{
		std::cerr << "[Warn ] Process cannot be resumed (does not exist)" << std::endl;
		return;
	}

	::kill(m_childPID, SIGCONT);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

int AsyncProcess::exitCode() const
{
	return m_exitCode;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void AsyncProcess::join() const
{
	BOOST_ASSERT_MSG(m_thread, "[Error] No thread to join");
	BOOST_ASSERT_MSG(m_thread->joinable(), "[Error] Thread not joinable");

	m_thread->join();

	BOOST_ASSERT(!isRunning());

	m_thread.reset(nullptr);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

}
