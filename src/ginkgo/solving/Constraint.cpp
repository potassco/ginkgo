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
	{"apply", "del", "holds", "terminal"};

////////////////////////////////////////////////////////////////////////////////////////////////////

Range<size_t> computeTimeRange(const Literals &literals)
{
	// Currently, normalization only works for plasp-formatted encodings
	std::for_each(literals.cbegin(), literals.cend(), [](const auto &literal)
	{
		BOOST_ASSERT_MSG(supportedTimeIdentifiers.find(*literal.name())
			!= supportedTimeIdentifiers.end(), "Identifier unsupported");
	});

	Range<size_t> timeRange{std::numeric_limits<size_t>::max(), std::numeric_limits<size_t>::min()};

	for (const auto &literal : literals)
	{
		BOOST_ASSERT(!literal.symbol()->clingoSymbol.arguments().empty());

		const auto &timeArgument = literal.symbol()->clingoSymbol.arguments().back();
		const size_t time = timeArgument.number();

		// Actions require at least one preceding time step in order to check preconditions
		if (std::strcmp(literal.symbol()->clingoSymbol.name(), "apply") == 0 || std::strcmp(literal.symbol()->clingoSymbol.name(), "del") == 0)
			timeRange.min = std::min(timeRange.min, time - 1);
		else
			timeRange.min = std::min(timeRange.min, time);

		timeRange.max = std::max(timeRange.max, time);
	}

	return timeRange;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

}
