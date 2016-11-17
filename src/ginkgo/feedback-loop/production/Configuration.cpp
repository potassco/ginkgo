#include <ginkgo/feedback-loop/production/Configuration.h>

#include <iostream>
#include <sstream>

#include <ginkgo/utils/Utils.h>

namespace ginkgo
{
namespace feedbackLoop
{
namespace production
{

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Configuration
//
////////////////////////////////////////////////////////////////////////////////////////////////////

Configuration<Plain>::Configuration()
{
	horizon = 0;
	proofMethod = ProofMethod::StateWise;
	minimizationStrategy = MinimizationStrategy::NoMinimization;
	fluentClosureUsage = FluentClosureUsage::NoFluentClosure;
	constraintsToExtract = 128;
	constraintsToProve = 1;
	maxDegree = std::numeric_limits<decltype(maxDegree)>::max();
	maxNumberOfLiterals = std::numeric_limits<decltype(maxNumberOfLiterals)>::max();
	extractionTimeout = std::chrono::seconds(10);
	hypothesisTestingTimeout = std::chrono::seconds(10);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Configuration<Plain> Configuration<Plain>::fromJSON(const Json::Value &json)
{
	Configuration<Plain> configuration;

	const auto &jsonInputFiles = json["InputFiles"];

	BOOST_ASSERT_MSG(jsonInputFiles.size() == 2,
		"Inconsistent number of input files, should be 2 (instance + encoding)");

	configuration.instance = jsonInputFiles[0].asString();
	configuration.domain = jsonInputFiles[1].asString();

	configuration.horizon = json["Horizon"].asUInt64();

	configuration.proofMethod = fromString<ProofMethod>(json["ProofMethod"].asString());
	configuration.minimizationStrategy = fromString<MinimizationStrategy>(json["MinimizationStrategy"].asString());
	configuration.fluentClosureUsage = fromString<FluentClosureUsage>(json["FluentClosureUsage"].asString());
	configuration.constraintsToExtract = json["ConstraintsToExtract"].asUInt64();
	configuration.constraintsToProve = json["ConstraintsToProve"].asUInt64();
	configuration.maxDegree = json["MaxDegree"].asUInt64();
	configuration.maxNumberOfLiterals = json["MaxNumberOfLiterals"].asUInt64();
	configuration.extractionTimeout = std::chrono::seconds(json["ExtractionTimeout"].asUInt64());
	configuration.hypothesisTestingTimeout = std::chrono::seconds(json["HypothesisTestingTimeout"].asUInt64());

	return configuration;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Json::Value Configuration<Plain>::toJSON() const
{
	Json::Value json;

	auto &jsonInputFiles = json["InputFiles"];

	jsonInputFiles = Json::arrayValue;
	jsonInputFiles.append(instance.string());
	jsonInputFiles.append(domain.string());

	json["Horizon"] = static_cast<Json::UInt64>(horizon);
	json["ProofMethod"] = toString(proofMethod);
	json["MinimizationStrategy"] = toString(minimizationStrategy);
	json["FluentClosureUsage"] = toString(fluentClosureUsage);
	json["ConstraintsToExtract"] = static_cast<Json::UInt64>(constraintsToExtract);
	json["ConstraintsToProve"] = static_cast<Json::UInt64>(constraintsToProve);
	json["MaxDegree"] = static_cast<Json::UInt64>(maxDegree);
	json["MaxNumberOfLiterals"] = static_cast<Json::UInt64>(maxNumberOfLiterals);
	json["ExtractionTimeout"] = static_cast<Json::UInt64>(std::chrono::duration_cast<std::chrono::seconds>(extractionTimeout).count());
	json["HypothesisTestingTimeout"] = static_cast<Json::UInt64>(std::chrono::duration_cast<std::chrono::seconds>(hypothesisTestingTimeout).count());

	return json;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

}
}
}
