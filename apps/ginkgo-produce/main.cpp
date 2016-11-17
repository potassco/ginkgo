#include <iostream>

#include <boost/program_options.hpp>

#include <ginkgo/utils/TextFile.h>
#include <ginkgo/utils/Utils.h>

#include <ginkgo/FeedbackLoopProduction.h>

int main(int argc, char **argv)
{
	namespace po = boost::program_options;

	po::options_description description("Allowed options");
	description.add_options()
		("help,h", "display this help message")
		("input,i", po::value<std::vector<std::string>>(), "Instance and domain files (both required)")
		("output,o", po::value<std::string>(), "Output prefix of the result files")
		("horizon", po::value<size_t>(), "Horizon (maximum time steps)")
		("proof-method", po::value<ginkgo::feedbackLoop::production::ProofMethod>()->default_value(ginkgo::feedbackLoop::production::ProofMethod::StateWise), "Proof method to use (StateWise, Induction)")
		("minimization-strategy", po::value<ginkgo::feedbackLoop::production::MinimizationStrategy>()->default_value(ginkgo::feedbackLoop::production::MinimizationStrategy::NoMinimization), "Clause minimization strategy (NoMinimization, SimpleMinimization, LinearMinimization)")
		("fluent-closure-usage", po::value<ginkgo::feedbackLoop::production::FluentClosureUsage>()->default_value(ginkgo::feedbackLoop::production::FluentClosureUsage::NoFluentClosure), "Usage of fluent closure (NoFluentClosure, UseFluentClosure)")
		("constraints-to-extract", po::value<size_t>()->default_value(1000), "Extract <n> constraints")
		("constraints-to-prove", po::value<size_t>()->default_value(0), "Finish after <n> proven constraints")
		("max-degree", po::value<size_t>()->default_value(5), "Maximum degree of hypotheses to test")
		("max-number-of-literals", po::value<size_t>()->default_value(10), "Maximum number of literals of hypotheses to test")
		("extraction-timeout", po::value<double>()->default_value(600.0), "Knowledge extraction timeout (seconds)")
		("hypothesis-testing-timeout", po::value<size_t>()->default_value(10), "Hypothesis testing timeout (seconds)")
		("log-level", po::value<ginkgo::feedbackLoop::production::LogLevel>()->default_value(ginkgo::feedbackLoop::production::LogLevel::Normal), "Output (Debug = detailed output)");

	po::positional_options_description positionalOptionsDescription;
	positionalOptionsDescription.add("input", -1);

	po::variables_map variablesMap;

	try
	{
		po::store(po::command_line_parser(argc, argv)
			.options(description)
			.positional(positionalOptionsDescription)
			.run(),
			variablesMap);
		po::notify(variablesMap);
	}
	catch (const po::error &e)
	{
		std::cerr << e.what() << std::endl;
		std::cout << std::endl << description;
		return EXIT_FAILURE;
	}

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
	checkVariable("horizon", "Horizon (maximum time steps) unspecified");
	checkVariable("proof-method", "Proof method unspecified");
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
	auto environment = std::make_unique<ginkgo::feedbackLoop::production::Environment>(variablesMap["output"].as<std::string>());
	environment->setLogLevel(variablesMap["log-level"].as<ginkgo::feedbackLoop::production::LogLevel>());

	auto configuration = std::make_unique<ginkgo::feedbackLoop::production::Configuration<ginkgo::Plain>>();
	configuration->horizon = variablesMap["horizon"].as<size_t>();
	configuration->proofMethod = variablesMap["proof-method"].as<ginkgo::feedbackLoop::production::ProofMethod>();
	configuration->minimizationStrategy = variablesMap["minimization-strategy"].as<ginkgo::feedbackLoop::production::MinimizationStrategy>();
	configuration->fluentClosureUsage = variablesMap["fluent-closure-usage"].as<ginkgo::feedbackLoop::production::FluentClosureUsage>();
	configuration->constraintsToExtract = variablesMap["constraints-to-extract"].as<size_t>();
	configuration->constraintsToProve = variablesMap["constraints-to-prove"].as<size_t>();
	configuration->maxDegree = variablesMap["max-degree"].as<size_t>();
	configuration->maxNumberOfLiterals = variablesMap["max-number-of-literals"].as<size_t>();
	configuration->extractionTimeout = std::chrono::milliseconds(static_cast<std::chrono::milliseconds::rep>(variablesMap["extraction-timeout"].as<double>() * 1000));
	configuration->hypothesisTestingTimeout = std::chrono::seconds(variablesMap["hypothesis-testing-timeout"].as<size_t>());

	configuration->instance = inputFileNames[0];
	configuration->domain = inputFileNames[1];

	ginkgo::feedbackLoop::production::FeedbackLoop(std::move(environment), std::move(configuration)).run();

	return EXIT_SUCCESS;
}
