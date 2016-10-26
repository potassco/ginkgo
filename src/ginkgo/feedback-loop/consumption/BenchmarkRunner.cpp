#include <ginkgo/feedback-loop/consumption/BenchmarkRunner.h>

#include <iostream>
#include <json/json.h>

#include <ginkgo/utils/TextFile.h>
#include <ginkgo/feedback-loop/production/FeedbackLoop.h>

namespace ginkgo
{
namespace feedbackLoop
{
namespace consumption
{

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// BenchmarkRunner
//
////////////////////////////////////////////////////////////////////////////////////////////////////

BenchmarkRunner::BenchmarkRunner(std::unique_ptr<BenchmarkEnvironment> environment)
:	m_environment(std::move(environment))
{
	Json::Value productionStatistics;
	m_environment->productionStatisticsStream() >> productionStatistics;

	m_productionConfiguration = production::Configuration<Plain>::fromJSON(productionStatistics["Configuration"]);
	m_productionEvents = production::Events::fromJSON(productionStatistics["Events"]);

	for (const auto &inputFileName : {m_productionConfiguration.instance, m_productionConfiguration.domain})
	{
		TextFile inputFile(inputFileName.string());
		m_program << inputFile.read().rdbuf() << std::endl;
	};

	m_environment->productionStatisticsStream().close();

	std::string line;

	BOOST_ASSERT_MSG(m_environment->feedbackType() != FeedbackType::Unknown, "Feedback type unspecified");

	auto &constraintsStream = m_environment->constraintsStream();

	while (std::getline(constraintsStream, line))
		m_constraints.push_back(line);

	constraintsStream.close();

	std::cout << "[Info ] Using horizon as specified by command-line argument (" << m_environment->horizon() << ")" << std::endl;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void BenchmarkRunner::run()
{
	setlocale(LC_NUMERIC, "C");

	m_program.clear();
	m_program.seekg(0, std::ios::beg);

	// Proceed in exponential steps
	const double step = std::pow(2.0, 1.0 / 4.0);

	double i = 8.0;

	bool groundingTimeout = false;

	auto measure =
		[&](size_t numberOfConstraints)
		{
			m_program.clear();
			m_program.seekg(0, std::ios::beg);

			std::stringstream metaEncoding;
			metaEncoding
				<< "#const horizon=" << m_environment->horizon() << "." << std::endl
				<< production::FeedbackLoop::MetaEncoding << std::endl
				<< m_program.rdbuf() << std::endl;

			const auto availableConstraints = std::min(numberOfConstraints, m_constraints.size());

			for (size_t j = 0; j < availableConstraints; j++)
				metaEncoding << m_constraints[j] << std::endl;

			metaEncoding.clear();
			metaEncoding.seekg(0, std::ios::beg);

			const auto groundingStartTime = std::chrono::high_resolution_clock::now();

			// TODO: reimplement
		};

	// Measure solving time without feedback
	measure(0);

	// Measure solving times when adding n generalized constraints to the encoding
	while (true)
	{
		const size_t numberOfConstraints = std::round(i);

		const auto finished = numberOfConstraints > m_environment->maxNumberOfConstraints()
			|| (m_environment->maxNumberOfConstraints() == 0
				&& numberOfConstraints > m_constraints.size());

		if (finished)
			break;

		measure(numberOfConstraints);

		i *= step;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////

}
}
}
