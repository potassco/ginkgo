#include <iostream>

#include <boost/program_options.hpp>

#include <ginkgo/utils/TextFile.h>
#include <ginkgo/FeedbackLoopAnalysis.h>

int main(int argc, char **argv)
{
	namespace po = boost::program_options;

	po::options_description description("Allowed options");
	description.add_options()
		("help,h", "display this help message")
		("file-prefix,f", po::value<std::string>(), "Ginkgo statistics file prefix");

	po::variables_map variablesMap;
	po::store(po::parse_command_line(argc, argv, description), variablesMap);
	po::notify(variablesMap);

	if (variablesMap.count("help"))
	{
		std::cout << description;
		return EXIT_SUCCESS;
	}

	if (variablesMap.count("file-prefix") > 0)
	{
		const auto filePrefix = variablesMap["file-prefix"].as<std::string>();
		const auto productionStatisticsFile = filePrefix + ".stats-produce";
		const auto consumptionDirectStatisticsFile = filePrefix + ".stats-consume-direct";
		const auto consumptionGeneralizedStatisticsFile = filePrefix + ".stats-consume-generalized";

		if (boost::filesystem::is_regular_file(productionStatisticsFile))
		{
			const auto productionAnalysis = ginkgo::feedbackLoop::analysis::ProductionAnalysis<ginkgo::Plain>::fromRawFile(productionStatisticsFile, 36000.0);
			productionAnalysis.write(filePrefix + ".analysis-produce");
		}

		if (boost::filesystem::is_regular_file(consumptionDirectStatisticsFile))
		{
			const auto consumptionDirectAnalysis = ginkgo::feedbackLoop::analysis::ConsumptionAnalysis<ginkgo::Plain>::fromRawFile(consumptionDirectStatisticsFile, 36000.0);
			consumptionDirectAnalysis.write(filePrefix + ".analysis-consume-direct");
		}

		if (boost::filesystem::is_regular_file(consumptionGeneralizedStatisticsFile))
		{
			const auto consumptionGeneralizedAnalysis = ginkgo::feedbackLoop::analysis::ConsumptionAnalysis<ginkgo::Plain>::fromRawFile(consumptionGeneralizedStatisticsFile, 36000.0);
			consumptionGeneralizedAnalysis.write(filePrefix + ".analysis-consume-generalized");
		}
	}

	return EXIT_SUCCESS;
}
