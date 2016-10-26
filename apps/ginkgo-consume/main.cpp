#include <iostream>

#include <boost/program_options.hpp>

#include <ginkgo/solving/__deprecated__Literal.h>
#include <ginkgo/solving/__deprecated__Constraints.h>
#include <ginkgo/utils/TextFile.h>
#include <ginkgo/utils/Utils.h>

#include <ginkgo/FeedbackLoopConsumption.h>

int main(int argc, char **argv)
{
	namespace po = boost::program_options;

	po::options_description description("Allowed options");
	description.add_options()
		("help,h", "display this help message")
		("file-prefix,f", po::value<std::string>(), "Ginkgo statistics file prefix")
		("feedback-type", po::value<ginkgo::feedbackLoop::consumption::FeedbackType>(), "Feedback type (Direct, Generalized)")
		("horizon", po::value<size_t>(), "Horizon (maximum time steps)")
		("max-constraints", po::value<size_t>(), "Maximum number of constraints to test")
		("time-limit", po::value<size_t>(), "Time limit per clasp execution in seconds");

	po::variables_map variablesMap;
	po::store(po::parse_command_line(argc, argv, description), variablesMap);
	po::notify(variablesMap);

	if (variablesMap.count("help"))
	{
		std::cout << description;
		return EXIT_SUCCESS;
	}

	auto checkVariable = [&](const std::string &variable, const::std::string &errorMessage)
	{
		if (variablesMap.count(variable) > 0)
			return;

		std::cerr << "[Error] " << errorMessage << std::endl;
		std::cout << description;
		exit(EXIT_FAILURE);
	};

	checkVariable("file-prefix", "No statistics file prefix specified");
	checkVariable("feedback-type", "Feedback type unspecified");
	checkVariable("horizon", "Horizon (maximum time steps) unspecified");
	checkVariable("max-constraints", "Maximum number of constraints to test unspecified");
	checkVariable("time-limit", "Time limit unspecified");

	// Feedback loop benchmark environment
	auto benchmarkEnvironment = std::make_unique<ginkgo::feedbackLoop::consumption::BenchmarkEnvironment>(variablesMap["file-prefix"].as<std::string>());
	benchmarkEnvironment->setFeedbackType(variablesMap["feedback-type"].as<ginkgo::feedbackLoop::consumption::FeedbackType>());
	benchmarkEnvironment->setHorizon(variablesMap["horizon"].as<size_t>());
	benchmarkEnvironment->setMaxNumberOfConstraints(variablesMap["max-constraints"].as<size_t>());

	ginkgo::feedbackLoop::consumption::BenchmarkRunner(std::move(benchmarkEnvironment)).run();

	return EXIT_SUCCESS;
}
