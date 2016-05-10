#include <ginkgo/utils/Utils.h>

#include <sstream>
#include <algorithm>
#include <cstring>
#include <boost/assert.hpp>

namespace ginkgo
{

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Utils
//
////////////////////////////////////////////////////////////////////////////////////////////////////

size_t findMatchingRightParenthesis(const std::string &string, size_t leftParenthesisPosition)
{
	BOOST_ASSERT_MSG(string[leftParenthesisPosition] == '(', "Character is not a left parenthesis.");

	size_t count = 0;

	for (auto position = leftParenthesisPosition; position < string.size(); position++)
	{
		if (string[position] == '(')
			count++;

		if (string[position] == ')')
			count--;

		if (count == 0)
			return position;
	}

	return std::string::npos;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool isNumeric(char c)
{
	return c >= '0' && c <= '9';
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool isAlphanumeric(char c)
{
	if (c >= 'a' && c <= 'z')
		return true;

	if (c >= 'A' && c <= 'Z')
		return true;

	if (c >= '0' && c <= '9')
		return true;

	// Workaround to build constraints like a(T), b(T+2).
	if (c == '+')
		return true;

	return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

}
