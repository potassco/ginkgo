#ifndef __FEEDBACK_LOOP__PRODUCTION__CONFIGURATION_H
#define __FEEDBACK_LOOP__PRODUCTION__CONFIGURATION_H

#include <chrono>

#include <boost/filesystem.hpp>

#include <json/json.h>

#include <ginkgo/feedback-loop/production/ProofMethod.h>
#include <ginkgo/feedback-loop/production/MinimizationStrategy.h>
#include <ginkgo/feedback-loop/production/FluentClosureUsage.h>
#include <ginkgo/feedback-loop/production/LogLevel.h>
#include <ginkgo/utils/Statistics.h>

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

template<template<class> class S>
struct ConfigurationBase
{
	// Input files
	typename S<boost::filesystem::path>::Set instance;
	typename S<boost::filesystem::path>::Set domain;

	// Fixed planning horizon
	typename S<size_t>::Numerical horizon;
	// Selected proof method
	typename S<production::ProofMethod>::Set proofMethod;
	// Selected minimization strategy
	typename S<production::MinimizationStrategy>::Set minimizationStrategy;
	// Selected fluent closure usage
	typename S<production::FluentClosureUsage>::Set fluentClosureUsage;
	// Selected number of constraints to extract
	typename S<size_t>::Set constraintsToExtract;
	// Selected number of constraints to prove before termination
	typename S<size_t>::Set constraintsToProve;
	// Maximum degree of candidates to test
	typename S<size_t>::Set maxDegree;
	// Maximum number of literals of candidates to test
	typename S<size_t>::Set maxNumberOfLiterals;
	// Timeout applied to candidate validation
	typename S<std::chrono::duration<double>>::Set candidateTestingTimeout;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

template<template<class> class S>
struct Configuration : public ConfigurationBase<S>
{
};

////////////////////////////////////////////////////////////////////////////////////////////////////

template<>
struct Configuration<Plain> : public ConfigurationBase<Plain>
{
	Configuration<Plain>();

	static Configuration<Plain> fromJSON(const Json::Value &json);
	Json::Value toJSON() const;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

template<>
struct Configuration<Aggregated> : public ConfigurationBase<Aggregated>
{
	template <class Container, class Accessor, class Selector>
	static Configuration aggregate(const Container &plainConfigurations,
		Accessor accessor, Selector selector = [](const auto &){return true;});
};

////////////////////////////////////////////////////////////////////////////////////////////////////

template <class Container, class Accessor, class Selector>
Configuration<Aggregated> Configuration<Aggregated>::aggregate(
	const Container &plainConfigurations, Accessor accessor, Selector selector)
{
	Configuration<Aggregated> aggregatedConfiguration;

	aggregatedConfiguration.instance.aggregate(plainConfigurations, [&](const auto &e) {return accessor(e).instance;}, selector);
	aggregatedConfiguration.domain.aggregate(plainConfigurations, [&](const auto &e) {return accessor(e).domain;}, selector);

	aggregatedConfiguration.horizon.aggregate(plainConfigurations, [&](const auto &e) {return accessor(e).horizon;}, selector);
	aggregatedConfiguration.proofMethod.aggregate(plainConfigurations, [&](const auto &e) {return accessor(e).proofMethod;}, selector);
	aggregatedConfiguration.minimizationStrategy.aggregate(plainConfigurations, [&](const auto &e) {return accessor(e).minimizationStrategy;}, selector);
	aggregatedConfiguration.fluentClosureUsage.aggregate(plainConfigurations, [&](const auto &e) {return accessor(e).fluentClosureUsage;}, selector);
	aggregatedConfiguration.constraintsToExtract.aggregate(plainConfigurations, [&](const auto &e) {return accessor(e).constraintsToExtract;}, selector);
	aggregatedConfiguration.constraintsToProve.aggregate(plainConfigurations, [&](const auto &e) {return accessor(e).constraintsToProve;}, selector);
	aggregatedConfiguration.maxDegree.aggregate(plainConfigurations, [&](const auto &e) {return accessor(e).maxDegree;}, selector);
	aggregatedConfiguration.maxNumberOfLiterals.aggregate(plainConfigurations, [&](const auto &e) {return accessor(e).maxNumberOfLiterals;}, selector);
	aggregatedConfiguration.candidateTestingTimeout.aggregate(plainConfigurations, [&](const auto &e) {return accessor(e).candidateTestingTimeout;}, selector);

	return aggregatedConfiguration;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

}
}
}

#endif
