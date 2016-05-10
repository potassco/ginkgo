#include <ginkgo/feedback-loop/analysis/ConsumptionAnalysis.h>

#include <fstream>
#include <iostream>

#include <json/json.h>
#include <boost/assert.hpp>

#include <ginkgo/utils/Statistics.h>

namespace ginkgo
{
namespace feedbackLoop
{
namespace analysis
{

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// ConsumptionAnalysis
//
////////////////////////////////////////////////////////////////////////////////////////////////////

ConsumptionAnalysis<Plain> ConsumptionAnalysis<Plain>::fromRawFile(const boost::filesystem::path &path, double penalty)
{
	ConsumptionAnalysis consumptionAnalysis;

	BOOST_ASSERT(penalty > 0.0);

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Read file
	////////////////////////////////////////////////////////////////////////////////////////////////

	setlocale(LC_NUMERIC, "C");

	const auto filePrefix = path.parent_path();

	if (!boost::filesystem::is_regular_file(path))
		std::cerr << "[Error] File does not exist: " << path.string() << std::endl;

	const auto readMeasurement =
		[&](const auto &json)
		{
			const auto selectedConstraints = static_cast<size_t>(json["Ginkgo"]["SelectedConstraints"].asUInt64());

			TimeMeasurement timeMeasurement;

			if (json["Ginkgo"].get("GroundingTimeout", "False").asBool() == true
				|| json.get("TIME LIMIT", 0).asDouble() == 1)
			{
				timeMeasurement.timeout = true;
				timeMeasurement.time = penalty;
			}
			else
			{
				timeMeasurement.timeout = false;
				timeMeasurement.time = json["Time"]["Solve"].asDouble();
			}

			consumptionAnalysis.measurements.push_back({selectedConstraints, timeMeasurement});
		};

	try
	{
		// Read consumption statistics file
		std::cout << "[Info ] Reading consumption statistics from " << path.string() << std::endl;
		std::ifstream statisticsStream(path.string(), std::ifstream::binary);
		std::string line;

		Json::Reader reader;

		while (std::getline(statisticsStream, line))
		{
			Json::Value json;
			reader.parse(line, json);

			readMeasurement(json);
		}
	}
	catch (std::exception &e)
	{
		std::cerr << "[Error] " << e.what() << std::endl;
	}

	consumptionAnalysis.sort();
	consumptionAnalysis.penalty = penalty;

	return consumptionAnalysis;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

ConsumptionAnalysis<Plain> ConsumptionAnalysis<Plain>::fromAnalysisFile(const boost::filesystem::path &path)
{
	setlocale(LC_NUMERIC, "C");

	std::cout << "[Info ] Reading analysis from " << path.string() << std::endl;

	std::fstream file(path.string(), std::ios::in);

	Json::Value json;
	file >> json;

	file.close();

	ConsumptionAnalysis<Plain> consumptionAnalysis;
	const auto &jsonMeasurements = json["Measurements"];

	std::for_each(jsonMeasurements.begin(), jsonMeasurements.end(),
		[&](const auto &jsonMeasurement)
		{
			const auto selectedConstraints = static_cast<size_t>(jsonMeasurement["SelectedConstraints"].asUInt64());
			const auto timeout = jsonMeasurement["Timeout"].asBool();
			const auto time = jsonMeasurement["Time"].asDouble();

			consumptionAnalysis.measurements.push_back({selectedConstraints, {timeout, time}});
		});

	consumptionAnalysis.sort();
	consumptionAnalysis.penalty = json["Penalty"].asDouble();

	return consumptionAnalysis;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void ConsumptionAnalysis<Plain>::write(const boost::filesystem::path &path) const
{
	setlocale(LC_NUMERIC, "C");

	Json::Value json;
	auto &jsonMeasurements = json["Measurements"];
	jsonMeasurements = Json::arrayValue;

	json["Penalty"] = penalty;

	std::for_each(measurements.cbegin(), measurements.cend(),
		[&](const auto &measurement)
		{
			Json::Value jsonMeasurement;
			jsonMeasurement["SelectedConstraints"] = static_cast<Json::UInt64>(measurement.selectedConstraints);
			jsonMeasurement["Time"] = measurement.timeMeasurement.time;
			jsonMeasurement["Timeout"] = measurement.timeMeasurement.timeout;

			jsonMeasurements.append(jsonMeasurement);
		});

	std::fstream file(path.string(), std::ios::out);

	file << json;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

}
}
}
