#include <ginkgo/solving/ClaspOutputParsing.h>

#include <iostream>
#include <boost/assert.hpp>
#include <json/json.h>

namespace ginkgo
{

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// ClaspOutputParsing
//
////////////////////////////////////////////////////////////////////////////////////////////////////

float parseForSolvingTime(std::stringstream &claspOutput)
{
	const auto outputString = claspOutput.str();

	const auto pattern = "Solving: ";

	auto solvingTimeStart = outputString.find(pattern);

	if (solvingTimeStart == std::string::npos)
		return 0.0f;

	solvingTimeStart += sizeof(pattern);

	const auto solvingTimeEnd = outputString.find("s", solvingTimeStart);
	std::stringstream solvingTimeStream(outputString.substr(solvingTimeStart, solvingTimeEnd - solvingTimeStart));

	float solvingTime;
	solvingTimeStream >> solvingTime;

	return solvingTime;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Satisfiability parseForSatisfiability(std::stringstream &claspOutputJson)
{
	try
	{
		Json::Value claspOutput;
		claspOutputJson >> claspOutput;

		const auto result = claspOutput.get("Result", "");

		if (result == "UNSATISFIABLE")
			return Satisfiability::Unsatisfiable;
		if (result == "SATISFIABLE")
			return Satisfiability::Satisfiable;
	}
	catch (std::exception &e)
	{
		std::cout << "[Warn ] Could not parse clasp output for satisfiability" << std::endl;
	}

	return Satisfiability::Unknown;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool parseForErrors(std::stringstream &claspOutput)
{
	return claspOutput.str().find("***") != std::string::npos;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool parseForWarnings(std::stringstream &claspOutput)
{
	return claspOutput.str().find("% warning") != std::string::npos;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

}
