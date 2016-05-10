#include <iostream>

#include <boost/program_options.hpp>

#include <ginkgo/solving/Literal.h>
#include <ginkgo/solving/Constraints.h>
#include <ginkgo/solving/Clasp.h>
#include <ginkgo/solving/Gringo.h>
#include <ginkgo/utils/TextFile.h>
#include <ginkgo/utils/Utils.h>

#include <ginkgo/FeedbackLoopProduction.h>

#include <ginkgo/solving/AsyncProcess.h>

int main(int argc, char **argv)
{
	namespace po = boost::program_options;

	po::options_description description("Allowed options");
	description.add_options()
		("help,h", "display this help message")
		("input,i", po::value<std::vector<std::string>>(), "Instance and domain files (both required)")
		("output,o", po::value<std::string>(), "Output prefix of the result files")
		("clasp,c", po::value<std::string>(), "clasp binary (unmodified)")
		("xclasp,x", po::value<std::string>(), "xclasp binary (clasp with extensions for knowledge extraction)")
		("gringo,g", po::value<std::string>(), "gringo binary")
		("horizon", po::value<size_t>(), "Horizon (maximum time steps)")
		("proof-method", po::value<ginkgo::feedbackLoop::production::ProofMethod>(), "Proof method to use (StateWise, Induction)")
		("testing-policy", po::value<ginkgo::feedbackLoop::production::TestingPolicy>(), "Feedback constraint Testing policy (FindFirst, TestAll)")
		("minimization-strategy", po::value<ginkgo::feedbackLoop::production::MinimizationStrategy>(), "Clause minimization strategy (NoMinimization, SimpleMinimization, LinearMinimization)")
		("fluent-closure-usage", po::value<ginkgo::feedbackLoop::production::FluentClosureUsage>(), "Usage of fluent closure (NoFluentClosure, UseFluentClosure)")
		("constraints-to-extract", po::value<size_t>(), "Extract <n> constraints")
		("constraints-to-prove", po::value<size_t>(), "Finish after <n> proven constraints")
		("max-degree", po::value<size_t>(), "Maximum degree of hypotheses to test")
		("max-number-of-literals", po::value<size_t>(), "Maximum number of literals of hypotheses to test")
		("extraction-timeout", po::value<size_t>(), "Knowledge extraction timeout (seconds)")
		("hypothesis-testing-timeout", po::value<size_t>(), "Hypothesis testing timeout (seconds)")
		("log-level", po::value<ginkgo::feedbackLoop::production::LogLevel>()->default_value(ginkgo::feedbackLoop::production::LogLevel::Normal), "Output (Debug = detailed output)");

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

	checkVariable("input", "No instance encoding specified");
	checkVariable("output", "No output prefix specified");
	checkVariable("clasp", "clasp binary unspecified");
	checkVariable("xclasp", "xclasp binary unspecified");
	checkVariable("gringo", "gringo binary unspecified");
	checkVariable("horizon", "Horizon (maximum time steps) unspecified");
	checkVariable("proof-method", "Proof method unspecified");
	checkVariable("testing-policy", "Testing policy unspecified");
	checkVariable("minimization-strategy", "Minimization strategy unspecified");
	checkVariable("fluent-closure-usage", "Fluent closure usage unspecified");
	checkVariable("constraints-to-extract", "Number of constraints to extract unspecified");
	checkVariable("constraints-to-prove", "Number of constraints to prove unspecified");
	checkVariable("max-degree", "Maximum degree of literals unspecified");
	checkVariable("max-number-of-literals", "Maximum number of literals unspecified");
	checkVariable("extraction-timeout", "Knowledge extraction timeout unspecified");
	checkVariable("hypothesis-testing-timeout", "Hypothesis testing timeout unspecified");

	const auto inputFileNames = variablesMap["input"].as<std::vector<std::string>>();

	if (inputFileNames.size() != 2)
	{
		std::cerr << "[Error] 2 input files required (first instance, then domain), " << inputFileNames.size() << " given" << std::endl;
		std::cout << description;
		exit(EXIT_FAILURE);
	}

	// Feedback loop environment
	ginkgo::AsyncProcess::Configuration claspConfiguration =
	{
		variablesMap["clasp"].as<std::string>(),
		{"--quiet=2", "--time-limit=600", "--stats=2", "--outf=2"}
	};

	ginkgo::AsyncProcess::Configuration xclaspConfiguration =
	{
		variablesMap["xclasp"].as<std::string>(),
		{"--log-learnts", "--resolution-scheme=named", "--reverse-arcs=0", "--otfs=0",
			"--heuristic=domain", "--loops=no", "--dom-mod=1,16", "--quiet=2", "--time-limit=600",
			"--stats=2", "--outf=2"}
	};

	ginkgo::AsyncProcess::Configuration gringoConfiguration =
	{
		variablesMap["gringo"].as<std::string>(),
		{}
	};

	auto environment = std::make_unique<ginkgo::feedbackLoop::production::Environment>(variablesMap["output"].as<std::string>());
	environment->setClaspConfiguration(claspConfiguration);
	environment->setXclaspConfiguration(xclaspConfiguration);
	environment->setGringoConfiguration(gringoConfiguration);
	environment->setLogLevel(variablesMap["log-level"].as<ginkgo::feedbackLoop::production::LogLevel>());

	auto configuration = std::make_unique<ginkgo::feedbackLoop::production::Configuration<ginkgo::Plain>>();
	configuration->horizon = variablesMap["horizon"].as<size_t>();
	configuration->proofMethod = variablesMap["proof-method"].as<ginkgo::feedbackLoop::production::ProofMethod>();
	configuration->testingPolicy = variablesMap["testing-policy"].as<ginkgo::feedbackLoop::production::TestingPolicy>();
	configuration->minimizationStrategy = variablesMap["minimization-strategy"].as<ginkgo::feedbackLoop::production::MinimizationStrategy>();
	configuration->fluentClosureUsage = variablesMap["fluent-closure-usage"].as<ginkgo::feedbackLoop::production::FluentClosureUsage>();
	configuration->constraintsToExtract = variablesMap["constraints-to-extract"].as<size_t>();
	configuration->constraintsToProve = variablesMap["constraints-to-prove"].as<size_t>();
	configuration->maxDegree = variablesMap["max-degree"].as<size_t>();
	configuration->maxNumberOfLiterals = variablesMap["max-number-of-literals"].as<size_t>();
	configuration->extractionTimeout = std::chrono::seconds(variablesMap["extraction-timeout"].as<size_t>());
	configuration->hypothesisTestingTimeout = std::chrono::seconds(variablesMap["hypothesis-testing-timeout"].as<size_t>());

	configuration->instance = inputFileNames[0];
	configuration->domain = inputFileNames[1];

	ginkgo::feedbackLoop::production::FeedbackLoop(std::move(environment), std::move(configuration)).run();

	return EXIT_SUCCESS;
}
