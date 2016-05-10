#include <ginkgo/solving/Process.h>

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

namespace ginkgo
{

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Process
//
////////////////////////////////////////////////////////////////////////////////////////////////////

Process::Process(std::string binary, const std::vector<std::string> &arguments)
:	m_binary{binary},
	m_exitCode{-1}
{
	auto convert = [](const std::string &s) -> char *
	{
	   char *pc = new char[s.size() + 1];
	   std::strcpy(pc, s.c_str());
	   return pc;
	};

	m_arguments.push_back(convert(binary));
	std::transform(arguments.begin(), arguments.end(), std::back_inserter(m_arguments), convert);
	m_arguments.push_back(nullptr);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Process::~Process()
{
	for (const auto &argument : m_arguments)
		delete[] argument;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Process::run(std::istream &input, bool reportErrors)
{
	m_stdout.str(std::string());
	m_stdout.clear();
	m_stderr.str(std::string());
	m_stderr.clear();

	// Pipe for stdin
	if (pipe(m_inPipe) == -1 || pipe(m_outPipe) == -1 || pipe(m_errPipe) == -1)
		exit(EXIT_FAILURE);

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
		runParentProcess(input, reportErrors);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Process::run(std::istream &input, const std::vector<std::string> &additionalArguments, bool reportErrors)
{
	auto convert = [](const std::string &s) -> char *
	{
	   char *pc = new char[s.size() + 1];
	   std::strcpy(pc, s.c_str());
	   return pc;
	};

	// Pop the nullptr
	m_arguments.pop_back();
	// Push the additional arguments
	std::transform(additionalArguments.begin(), additionalArguments.end(), std::back_inserter(m_arguments), convert);
	// Push the nullptr
	m_arguments.push_back(nullptr);

	run(input, reportErrors);

	// Pop the nullptr
	m_arguments.pop_back();
	// Pop and delete the additional arguments
	for (size_t i = 0; i < additionalArguments.size(); i++)
	{
		delete m_arguments.back();
		m_arguments.pop_back();
	}
	// Push the nullptr
	m_arguments.push_back(nullptr);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Process::runParentProcess(std::istream &input, bool reportErrors)
{
	const int &childReadIn = m_inPipe[0];
	const int &parentWriteIn = m_inPipe[1];
	const int &parentReadOut = m_outPipe[0];
	const int &childWriteOut = m_outPipe[1];
	const int &parentReadErr = m_errPipe[0];
	const int &childWriteErr = m_errPipe[1];

	close(childReadIn);
	close(childWriteOut);
	close(childWriteErr);

	std::stringstream::char_type buffer[1024];

	const auto maxFD = std::max(std::max(parentReadOut, parentReadErr), parentWriteIn);

	auto handleWrite = [&]()
	{
		if (!input.good())
		{
			close(parentWriteIn);
			return false;
		}

		input.read(buffer, sizeof(buffer));
		const auto count = input.gcount();

		const auto written = write(parentWriteIn, buffer, count);

		if (written != count)
			std::cerr << "[Error] Couldn't write entire buffer" << std::endl;

		return true;
	};

	auto handleRead = [&](int fileDescriptor, std::ostream &outputStream, std::mutex &outputStreamMutex)
	{
		const auto count = read(fileDescriptor, buffer, sizeof(buffer));

		if (count == 0)
		{
			close(fileDescriptor);
			return false;
		}

		std::lock_guard<std::mutex> lock(outputStreamMutex);
		outputStream.write(buffer, count).flush();
		return true;
	};

	bool parentReadOutActive = true;
	bool parentReadErrActive = true;
	bool parentWriteInActive = true;

	auto waitForInput = [&]()
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
			parentReadOutActive &= handleRead(parentReadOut, m_stdout, m_stdoutMutex);

		if (FD_ISSET(parentReadErr, &readFDSet))
			parentReadErrActive &= handleRead(parentReadErr, m_stderr, m_stderrMutex);

		if (FD_ISSET(parentWriteIn, &writeFDSet))
			parentWriteInActive &= handleWrite();

		return parentReadOutActive || parentReadErrActive || parentWriteInActive;
	};

	while (waitForInput());

	auto finishReading = [&](int fileDescriptor, std::ostream &outputStream, std::mutex &outputStreamMutex)
	{
		if (fcntl(fileDescriptor, F_GETFD) == -1 && errno == EBADF)
			return;

		while (handleRead(fileDescriptor, outputStream, outputStreamMutex));
	};

	finishReading(parentReadOut, m_stdout, m_stdoutMutex);
	finishReading(parentReadErr, m_stderr, m_stderrMutex);

	close(parentReadOut);
	close(parentReadErr);

	int status = 0;
	waitpid(m_childPID, &status, 0);

	m_exitCode = WEXITSTATUS(status);

	if (reportErrors && !isStderrEmpty())
	{
		std::cerr << "\033[1;31m[Error] Error occurred while executing " << m_binary << ":" << std::endl;
		std::cerr << m_stderr.rdbuf() << "\033[0m" << std::endl;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Process::runChildProcess()
{
	const int &childReadIn = m_inPipe[0];
	const int &parentWriteIn = m_inPipe[1];
	const int &parentReadOut = m_outPipe[0];
	const int &childWriteOut = m_outPipe[1];
	const int &parentReadErr = m_errPipe[0];
	const int &childWriteErr = m_errPipe[1];

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

	_exit(execvp(m_arguments[0], m_arguments.data()));
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::stringstream &Process::stdout()
{
	return m_stdout;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::stringstream &Process::stderr()
{
	return m_stderr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool Process::isStdoutEmpty() const
{
	return m_stdout.str().empty();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool Process::isStderrEmpty() const
{
	return m_stderr.str().empty();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

int Process::exitCode() const
{
	return m_exitCode;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

}
