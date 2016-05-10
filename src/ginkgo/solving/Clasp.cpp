#include <ginkgo/solving/Clasp.h>

#include <iostream>
#include <boost/assert.hpp>

namespace ginkgo
{

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Clasp
//
////////////////////////////////////////////////////////////////////////////////////////////////////

Clasp::Clasp(boost::filesystem::path binary, std::vector<std::string> arguments)
:	Process(binary.string(), arguments)
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////

float Clasp::solvingTime() const
{
	const auto pattern = "Solving: ";

	auto solvingTimeStart = m_stdout.str().find(pattern);

	if (solvingTimeStart == std::string::npos)
		return 0.0f;

	solvingTimeStart += sizeof(pattern);

	const auto solvingTimeEnd = m_stdout.str().find("s", solvingTimeStart);
	std::stringstream solvingTimeStream(m_stdout.str().substr(solvingTimeStart, solvingTimeEnd - solvingTimeStart));

	float solvingTime;
	solvingTimeStream >> solvingTime;

	return solvingTime;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Satisfiability Clasp::satisfiability() const
{
	if (m_stdout.str().find("UNSATISFIABLE") != std::string::npos)
		return Satisfiability::Unsatisfiable;

	if (m_stdout.str().find("SATISFIABLE") != std::string::npos)
		return Satisfiability::Satisfiable;

	std::cout << "[Warn ] Could not parse for satisfiability" << std::endl;
	return Satisfiability::Unknown;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool Clasp::hasError() const
{
	return m_stderr.str().find("***") != std::string::npos;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

}
