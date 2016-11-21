#include <ginkgo/solving/Constraint.h>

#include <set>

#include <boost/assert.hpp>

namespace ginkgo
{

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Constraint
//
////////////////////////////////////////////////////////////////////////////////////////////////////

static const std::set<std::string> supportedTimeIdentifiers =
	{"caused", "holds", "modified", "occurs"};

////////////////////////////////////////////////////////////////////////////////////////////////////

Range<size_t> computeTimeRange(const Literals &literals)
{
	// TODO: handle nontemporal literals appropriately

	// Currently, normalization only works for plasp-formatted encodings
	std::for_each(literals.cbegin(), literals.cend(),
		[](const auto &literal)
		{
			BOOST_ASSERT_MSG(supportedTimeIdentifiers.find(literal.symbol.name())
				!= supportedTimeIdentifiers.end(), "Identifier unsupported");
		});

	Range<size_t> timeRange{std::numeric_limits<size_t>::max(), std::numeric_limits<size_t>::min()};

	for (const auto &literal : literals)
	{
		const auto symbol = literal.symbol;

		BOOST_ASSERT(!symbol.arguments().empty());

		const auto &timeArgument = symbol.arguments().back();
		const size_t time = timeArgument.number();

		// TODO: add unit test
		// Actions require at least one preceding time step in order to check preconditions
		if (std::strcmp(symbol.name(), "occurs") == 0
		    || std::strcmp(symbol.name(), "caused") == 0
		    || std::strcmp(symbol.name(), "modified") == 0)
		{
			timeRange.min = std::min(timeRange.min, time - 1);
		}
		else
			timeRange.min = std::min(timeRange.min, time);

		timeRange.max = std::max(timeRange.max, time);
	}

	return timeRange;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool subsumes(const Literals &lhs, const Literals &rhs)
{
	return std::includes(rhs.cbegin(), rhs.cend(), lhs.cbegin(), lhs.cend());
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool subsumes(const Literals &lhs, const Literals &rhs, int lhsOffset, int rhsOffset)
{
	if (lhsOffset == 0 && rhsOffset == 0)
		return subsumes(lhs, rhs);

	// TODO: implement more efficiently
	const auto equalsShifted =
		[lhsOffset, rhsOffset](const auto &lhs, const auto &rhs)
		{
			if (lhs.sign != rhs.sign)
				return false;

			const auto lhsSymbol = lhs.symbol;
			const auto rhsSymbol = rhs.symbol;

			if (lhsSymbol.name() != rhsSymbol.name())
				return false;

			if (lhsSymbol.arguments().size() != rhsSymbol.arguments().size())
				return false;

			for (size_t i = 0; i < lhsSymbol.arguments().size() - 1; i++)
				if (lhsSymbol.arguments()[i] != rhsSymbol.arguments()[i])
					return false;

			const auto lhsTimeArgument = lhsSymbol.arguments().back().number();
			const auto rhsTimeArgument = rhsSymbol.arguments().back().number();

			return lhsTimeArgument + lhsOffset == rhsTimeArgument + rhsOffset;
		};

	for (const auto &lhsLiteral : lhs)
	{
		const auto matchingLiteral = std::find_if(rhs.cbegin(), rhs.cend(),
			[&](const auto rhsLiteral)
			{
				return equalsShifted(lhsLiteral, rhsLiteral);
			});

		if (matchingLiteral == rhs.cend())
			return false;
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool contains(const Literals &literals, const char *predicateName)
{
	return std::find_if(literals.cbegin(), literals.cend(),
		[&](const auto literal)
		{
			return std::strcmp(predicateName, literal.symbol.name()) == 0;
		}) != literals.cend();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

}
