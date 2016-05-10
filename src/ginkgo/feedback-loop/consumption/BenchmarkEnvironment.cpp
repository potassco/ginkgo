#include <ginkgo/feedback-loop/consumption/BenchmarkEnvironment.h>

#include <iostream>

namespace ginkgo
{
namespace feedbackLoop
{
namespace consumption
{

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// BenchmarkEnvironment
//
////////////////////////////////////////////////////////////////////////////////////////////////////

// TODO: define file endings in central place
BenchmarkEnvironment::BenchmarkEnvironment(boost::filesystem::path filePrefix)
:	m_filePrefix{filePrefix},
	m_feedbackType{FeedbackType::Unknown},
	m_horizon{0},
	m_maxNumberOfConstraints{0},
	m_productionStatisticsStream(m_filePrefix.string() + ".stats-produce", std::ios::in)
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void BenchmarkEnvironment::setClaspConfiguration(AsyncProcess::Configuration claspConfiguration)
{
	m_claspConfiguration = claspConfiguration;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

const AsyncProcess::Configuration &BenchmarkEnvironment::claspConfiguration() const
{
	return m_claspConfiguration;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void BenchmarkEnvironment::setGringoConfiguration(AsyncProcess::Configuration gringoConfiguration)
{
	m_gringoConfiguration = gringoConfiguration;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

const AsyncProcess::Configuration &BenchmarkEnvironment::gringoConfiguration() const
{
	return m_gringoConfiguration;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void BenchmarkEnvironment::setFeedbackType(FeedbackType feedbackType)
{
	m_feedbackType = feedbackType;

	if (m_feedbackType == FeedbackType::Direct)
	{
		m_constraintsStream = std::ifstream(m_filePrefix.string() + ".constraints-direct", std::ios::in);
		m_consumptionStatisticsStream = std::ofstream(m_filePrefix.string() + ".stats-consume-direct", std::ios::out);
	}
	else if (m_feedbackType == FeedbackType::Generalized)
	{
		m_constraintsStream = std::ifstream(m_filePrefix.string() + ".constraints-generalized", std::ios::in);
		m_consumptionStatisticsStream = std::ofstream(m_filePrefix.string() + ".stats-consume-generalized", std::ios::out);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////

FeedbackType BenchmarkEnvironment::feedbackType() const
{
	return m_feedbackType;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void BenchmarkEnvironment::setHorizon(size_t horizon)
{
	m_horizon = horizon;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

size_t BenchmarkEnvironment::horizon() const
{
	return m_horizon;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void BenchmarkEnvironment::setMaxNumberOfConstraints(size_t maxNumberOfConstraints)
{
	m_maxNumberOfConstraints = maxNumberOfConstraints;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

size_t BenchmarkEnvironment::maxNumberOfConstraints() const
{
	return m_maxNumberOfConstraints;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::ifstream &BenchmarkEnvironment::constraintsStream()
{
	return m_constraintsStream;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::ifstream &BenchmarkEnvironment::productionStatisticsStream()
{
	return m_productionStatisticsStream;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::ofstream &BenchmarkEnvironment::consumptionStatisticsStream()
{
	return m_consumptionStatisticsStream;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

}
}
}
