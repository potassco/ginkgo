#ifndef __UTILS__STATISTICS_H
#define __UTILS__STATISTICS_H

#include <iostream>
#include <functional>
#include <algorithm>
#include <vector>
#include <cmath>
#include <numeric>

#include <boost/assert.hpp>

namespace ginkgo
{

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Statistics
//
////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename T>
struct Sum
{
	using Type = T;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

template <>
struct Sum<bool>
{
	using Type = size_t;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename T>
struct MeanAndStdDev
{
	double mean;

	typename Sum<T>::Type sum;

	T min;
	T max;

	double stdDev;
	double stdErrOfMean;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename T>
struct Plain
{
	using Set = T;

	using Numerical = T;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename T>
struct Aggregated
{
	struct Set
	{
		template <class Container>
		void aggregate(const Container &values,
			const std::function<T (const typename Container::value_type &)> accessor,
			const std::function<bool (const typename Container::value_type &)> selector = [](const typename Container::value_type &){return true;})
		{
			std::for_each(values.cbegin(), values.cend(),
				[&](const auto &element)
				{
					if (!selector(element))
						return;

					const auto value = accessor(element);

					if (std::find(uniqueValues.cbegin(), uniqueValues.cend(), value) == uniqueValues.cend())
						uniqueValues.push_back(value);
				});

			std::sort(uniqueValues.begin(), uniqueValues.end());
		}

		std::vector<T> uniqueValues;
	};

	struct Numerical
	{
		template <typename Container, typename Accessor, typename Selector>
		void aggregate(const Container &values,
			Accessor accessor, Selector selector = [](const typename Container::value_type &){return true;},
			double minValue = std::numeric_limits<double>::min())
		{
			sampleSize = std::count_if(values.cbegin(), values.cend(), selector);

			BOOST_ASSERT(sampleSize > 0);

			const auto computeMeanAndStdDev =
				[&](auto accessor,
					auto &meanAndStdDev)
				{
					using InnerT = decltype(accessor(*values.cbegin()));
					using SumT = typename Sum<InnerT>::Type;

					meanAndStdDev.sum = std::accumulate(values.cbegin(), values.cend(), static_cast<SumT>(0),
						[&](SumT sum, const auto &element)
						{
							return sum + (selector(element) ? static_cast<SumT>(accessor(element)) : static_cast<SumT>(0));
						});

					meanAndStdDev.min = std::accumulate(values.cbegin(), values.cend(), std::numeric_limits<InnerT>::max(),
						[&](InnerT min, const auto &element)
						{
							return selector(element) ? std::min(min, static_cast<InnerT>(accessor(element))) : min;
						});

					meanAndStdDev.max = std::accumulate(values.cbegin(), values.cend(), std::numeric_limits<InnerT>::min(),
						[&](InnerT max, const auto &element)
						{
							return selector(element) ? std::max(max, static_cast<InnerT>(accessor(element))) : max;
						});

					meanAndStdDev.mean = static_cast<double>(meanAndStdDev.sum) / sampleSize;

					const double squaredDistanceSum = std::accumulate(values.cbegin(), values.cend(), 0.0,
						[&](double squaredDistanceSum, const auto &element)
						{
							if (!selector(element))
								return squaredDistanceSum;

							const auto value = accessor(element);
							return squaredDistanceSum + (value - meanAndStdDev.mean) * (value - meanAndStdDev.mean);
						});

					// Corrected sample standard deviation
					meanAndStdDev.stdDev = std::sqrt(squaredDistanceSum / (static_cast<double>(sampleSize - 1)));
					meanAndStdDev.stdErrOfMean = meanAndStdDev.stdDev / std::sqrt(static_cast<double>(sampleSize));
				};

			computeMeanAndStdDev(accessor, arithmetic);

			computeMeanAndStdDev(
				[&](const auto &element)
				{
					return std::max(minValue, static_cast<double>(std::log(accessor(element))));
				},
				geometric);

			geometric.mean = std::exp(geometric.mean);
			geometric.min = std::exp(geometric.min);
			geometric.max = std::exp(geometric.max);
		}

		MeanAndStdDev<T> arithmetic;
		MeanAndStdDev<double> geometric;

		size_t sampleSize;
	};
};

////////////////////////////////////////////////////////////////////////////////////////////////////

template <class Container>
void computeMeanStdDevAndCI(const Container &values, double criticalValue, double &mean, double &stdDev,
	double &ciLow, double &ciHigh,
	const std::function<double (const typename Container::value_type &)> &accessor,
	const std::function<bool (const typename Container::value_type &)> &selector = [](const typename Container::value_type &){return true;})
{
	const auto numberOfElements = std::count_if(values.cbegin(), values.cend(), selector);

	BOOST_ASSERT(numberOfElements > 0);

	const double sum = std::accumulate(values.cbegin(), values.cend(), 0.0, [&](double sum, const typename Container::value_type &element)
	{
		return sum + (selector(element) ? accessor(element) : 0.0);
	});

	mean = sum / numberOfElements;

	if (numberOfElements == 1)
	{
		stdDev = 0.0;
		ciLow = mean;
		ciHigh = mean;
		return;
	}

	const double squaredDistanceSum = std::accumulate(values.cbegin(), values.cend(), 0.0, [&](double squaredDistanceSum, const typename Container::value_type &element)
	{
		if (!selector(element))
			return squaredDistanceSum;

		const auto value = accessor(element);
		return squaredDistanceSum + (value - mean) * (value - mean);
	});

	// Corrected sample standard deviation
	stdDev = std::sqrt(squaredDistanceSum / (numberOfElements - 1));
	const auto stdErr = stdDev / std::sqrt(numberOfElements);

	ciLow = mean - criticalValue * stdErr;
	ciHigh = mean + criticalValue * stdErr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename _Tp, typename _Alloc = std::allocator<_Tp>>
void computeMeanStdDevAndCI(const std::vector<_Tp, _Alloc> &values, double criticalValue, double &mean, double &stdDev,
	double &ciLow, double &ciHigh,
	const std::function<bool (const _Tp &)> &selector = [](const _Tp &){return true;})
{
	computeMeanStdDevAndCI<std::vector<_Tp, _Alloc>>(values, criticalValue, mean, stdDev, ciLow, ciHigh, [](const _Tp &element){return element;}, selector);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename _Key, typename _Tp, typename _Compare = std::less<_Key>,
	typename _Alloc = std::allocator<std::pair<const _Key, _Tp>>>
void computeMeanStdDevAndCI(const std::map<_Key, _Tp, _Compare, _Alloc> &values, double criticalValue, double &mean, double &stdDev,
	double &ciLow, double &ciHigh,
	const std::function<bool (const _Tp &)> &selector = [](const _Tp &){return true;})
{
	using MapType = std::map<_Key, _Tp, _Compare, _Alloc>;

	computeMeanStdDevAndCI<MapType>(values, criticalValue, mean, stdDev, ciLow, ciHigh, [](const typename MapType::value_type &element){return element.second;},
		[&selector](const typename MapType::value_type &element){return selector(element.second);});
}

////////////////////////////////////////////////////////////////////////////////////////////////////

template <class Container>
void computeGeometricMeanStdDevAndCI(const Container &values, double minValue, double criticalValue, double &mean,
	double &stdDev, double &ciLow, double &ciHigh,
	const std::function<double (const typename Container::value_type &)> &accessor,
	const std::function<bool (const typename Container::value_type &)> &selector = [](const typename Container::value_type &){return true;})
{
	double logMean;
	double logStdDev;
	double logCILow;
	double logCIHigh;

	const auto accessLogValue =
		[&accessor, &minValue](const auto &element)
		{
			return std::log(std::max(minValue, accessor(element)));
		};

	computeMeanStdDevAndCI(values, criticalValue, logMean, logStdDev, logCILow, logCIHigh,
		accessLogValue, selector);

	mean = std::exp(logMean);
	stdDev = std::exp(logStdDev);
	ciLow = std::exp(logCILow);
	ciHigh = std::exp(logCIHigh);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename _Tp, typename _Alloc = std::allocator<_Tp>>
void computeGeometricMeanStdDevAndCI(const std::vector<_Tp, _Alloc> &values, double minValue, double criticalValue,
	double &mean, double &stdDev, double &ciLow, double &ciHigh,
	const std::function<bool (const _Tp &)> &selector = [](const _Tp &){return true;})
{
	computeGeometricMeanStdDevAndCI<std::vector<_Tp, _Alloc>>(values, minValue, criticalValue, mean, stdDev, ciLow, ciHigh, [](const _Tp &element){return element;}, selector);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename _Key, typename _Tp, typename _Compare = std::less<_Key>,
	typename _Alloc = std::allocator<std::pair<const _Key, _Tp>>>
void computeGeometricMeanStdDevAndCI(const std::map<_Key, _Tp, _Compare, _Alloc> &values,  double minValue, double criticalValue,
	double &mean, double &stdDev, double &ciLow, double &ciHigh,
	const std::function<bool (const _Tp &)> &selector = [](const _Tp &){return true;})
{
	using MapType = std::map<_Key, _Tp, _Compare, _Alloc>;

	computeGeometricMeanStdDevAndCI<MapType>(values, minValue, criticalValue, mean, stdDev, ciLow, ciHigh, [](const typename MapType::value_type &element){return element.second;},
		[&selector](const typename MapType::value_type &element){return selector(element.second);});
}

////////////////////////////////////////////////////////////////////////////////////////////////////

}

#endif

