#ifndef __UTILS__UTILS_H
#define __UTILS__UTILS_H

#include <string>
#include <sstream>
#include <iomanip>
#include <set>

namespace ginkgo
{

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Utils
//
////////////////////////////////////////////////////////////////////////////////////////////////////

size_t findMatchingRightParenthesis(const std::string &string, size_t leftParenthesisPosition);
bool isNumeric(char c);
bool isAlphanumeric(char c);

auto identity =
	[](auto predicate)
	{
		return predicate;
	};

auto logicalNot =
	[](auto predicate)
	{
		return [&predicate](auto... args)
		{
			return !predicate(args...);
		};
	};

auto toString =
	[](const auto &x, size_t precision = 0)
	{
		std::stringstream s;
		s << std::fixed << std::setprecision(precision) << x;

		return s.str();
	};

template<typename T>
T fromString(const std::string &x)
{
	T result;

	std::stringstream s(x);
	s >> result;

	return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

}

#endif
