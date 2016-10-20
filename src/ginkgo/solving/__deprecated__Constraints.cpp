#include <ginkgo/solving/__deprecated__Constraints.h>

#include <sstream>
#include <algorithm>
#include <iostream>
#include <chrono>
#include <boost/assert.hpp>
#include <boost/bimap.hpp>
#include <boost/assign.hpp>

#include <ginkgo/utils/Utils.h>

namespace ginkgo
{
namespace deprecated
{

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Constraints
//
////////////////////////////////////////////////////////////////////////////////////////////////////

Constraints::Constraints(SymbolTable &symbolTable)
:	m_symbolTable(symbolTable)
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Constraints::Constraints(SymbolTable &symbolTable, const std::string &string)
:	m_currentSortKey{SortKey::Chronology},
	m_currentSortDirection{SortDirection::Ascending},
	m_symbolTable(symbolTable)
{
	std::stringstream stringStream(string);
	std::string line;

	size_t constraintID = 0;

	while (std::getline(stringStream, line, '\n'))
	{
		if (line.empty())
			continue;

		if (line.find("***") != std::string::npos)
		{
			std::cout << "[Warn ] Malformed input for constraints" << std::endl;
			continue;
		}

		push_back(std::make_shared<Constraint>(constraintID, line, m_symbolTable));

		constraintID++;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////

// TODO: Remove 'force' workaround
void Constraints::sortBy(SortKey sortKey, SortDirection sortDirection, bool force)
{
	if (!force && m_currentSortKey == sortKey && m_currentSortDirection == sortDirection)
		return;

	auto sortByChronology = [](const ConstraintPtr &a, const ConstraintPtr &b)
	{
		return a->id() < b->id();
	};

	auto sortBySize = [](const ConstraintPtr &a, const ConstraintPtr &b)
	{
		if (a->literals().size() == b->literals().size())
			return a->id() < b->id();

		return a->literals().size() < b->literals().size();
	};

	auto sortByLBD = [](const ConstraintPtr &a, const ConstraintPtr &b)
	{
		if (a->lbd() == b->lbd())
			return a->id() < b->id();

		return a->lbd() < b->lbd();
	};

	auto sortByTimeDegree = [](const ConstraintPtr &a, const ConstraintPtr &b)
	{
		const auto timeRangeA = a->timeRange();
		const auto timeRangeB = b->timeRange();
		const auto degreeA = std::get<1>(timeRangeA) - std::get<0>(timeRangeA);
		const auto degreeB = std::get<1>(timeRangeB) - std::get<0>(timeRangeB);

		if (degreeA == degreeB)
		{
			if (a->literals().size() == b->literals().size())
				return a->id() < b->id();

			return a->literals().size() < b->literals().size();
		}

		return degreeA < degreeB;
	};

	auto sort = [&](auto directionPredicate)
	{
		switch (sortKey)
		{
			case SortKey::Chronology:
				std::sort(begin(), end(), directionPredicate(sortByChronology));
				break;
			case SortKey::Size:
				std::sort(begin(), end(), directionPredicate(sortBySize));
				break;
			case SortKey::LBD:
				std::sort(begin(), end(), directionPredicate(sortByLBD));
				break;
			case SortKey::TimeDegree:
				std::sort(begin(), end(), directionPredicate(sortByTimeDegree));
				break;
			default:
				BOOST_ASSERT_MSG(false, "Invalid order key");
		}
	};

	if (sortDirection == SortDirection::Ascending)
		sort(identity);
	else
		sort(logicalNot);

	m_currentSortKey = sortKey;
	m_currentSortDirection = sortDirection;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Constraints::select(Order order, size_t number, Constraints &subset, size_t &beginIndex)
{
	if (subset.size() >= number)
		return;

	sortBy(order.first);

	auto addConstraintAndSimplify = [&](std::shared_ptr<Constraint> &constraint)
	{
		if (subset.subsumes(*constraint))
			return;

		subset.removeConstraintsSubsumedBy(*constraint);
		subset.push_back(constraint);
	};

	switch (order.second)
	{
		case SortDirection::Ascending:
		{
			auto i = begin() + beginIndex;

			while (subset.size() < number && i != end())
			{
				addConstraintAndSimplify(*i);
				i++;
				beginIndex++;
			}

			break;
		}

		case SortDirection::Descending:
		{
			auto i = rbegin() + beginIndex;

			while (subset.size() < number && i != rend())
			{
				addConstraintAndSimplify(*i);
				i++;
				beginIndex++;
			}

			break;
		}

		default:
			std::cerr << "[Warn ] Unknown sort direction" << std::endl;
			break;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////

float Constraints::averageNumberOfLiterals() const
{
	size_t numberOfLiterals = 0;

	std::for_each(cbegin(), cend(),
		[&](const auto &constraint)
		{
			numberOfLiterals += constraint->literals().size();
		});

	return (float)numberOfLiterals / size();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Constraints::removeConstraintsSubsumedBy(const Constraint &constraint)
{
	erase(std::remove_if(begin(), end(), [&](auto element)
	{
		return constraint.subsumes(*element);
	}), end());
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Constraints::removeConstraintsSubsumedBy(const GeneralizedConstraint &generalizedConstraint)
{
	erase(std::remove_if(begin(), end(), [&](auto element)
	{
		return generalizedConstraint.subsumes(*element);
	}), end());
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Constraints::removeConstraintsContainingIdentifier(const std::string &identifier)
{
	erase(std::remove_if(begin(), end(), [&](auto element)
	{
		return element->containsIdentifier(identifier);
	}), end());
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Constraints::removeConstraintsWithTooHighDegree(size_t maxDegree)
{
	erase(std::remove_if(begin(), end(), [&](auto element)
	{
		const auto timeRange = element->timeRange();
		const auto degree = std::get<1>(timeRange) - std::get<0>(timeRange);

		return degree > maxDegree;
	}), end());
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Constraints::removeConstraintsContainingTooManyLiterals(size_t maxNumberOfLiterals)
{
	erase(std::remove_if(begin(), end(), [&](auto element)
	{
		return element->literals().size() > maxNumberOfLiterals;
	}), end());
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Constraints::removeLiterals(const std::string &literalName)
{
	std::for_each(begin(), end(),
		[&](auto &constraint)
		{
			constraint->removeLiterals(literalName);
		});

	// Remove constraints that are empty now
	erase(std::remove_if(begin(), end(), [&](auto element)
	{
		return element->literals().empty();
	}), end());
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool Constraints::subsumes(const Constraint &constraint) const
{
	for (auto i = cbegin(); i != cend(); i++)
		if ((*i)->subsumes(constraint))
			return true;

	return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

size_t Constraints::size() const
{
	return m_constraints.size();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Constraints::clear()
{
	m_constraints.clear();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool Constraints::empty() const
{
	return m_constraints.empty();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Constraints::iterator Constraints::begin()
{
	return m_constraints.begin();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Constraints::iterator Constraints::end()
{
	return m_constraints.end();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Constraints::const_iterator Constraints::cbegin() const
{
	return m_constraints.cbegin();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Constraints::const_iterator Constraints::cend() const
{
	return m_constraints.cend();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Constraints::reverse_iterator Constraints::rbegin()
{
	return m_constraints.rbegin();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Constraints::reverse_iterator Constraints::rend()
{
	return m_constraints.rend();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Constraints::reference Constraints::back()
{
	return m_constraints.back();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Constraints::pop_back()
{
	m_constraints.pop_back();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Constraints::push_back(const Constraints::DataType::value_type &x)
{
	m_constraints.push_back(x);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Constraints::iterator Constraints::erase(iterator first, iterator last)
{
	return m_constraints.erase(first, last);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Constraints::iterator Constraints::erase(iterator position)
{
	return m_constraints.erase(position);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::ostream &operator<<(std::ostream &ostream, const Constraints &constraints)
{
	std::for_each(constraints.cbegin(), constraints.cend(),
		[&](const auto &constraint)
		{
			ostream << *constraint.get() << std::endl;
		});

	return ostream;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

using OrderNames = boost::bimap<Constraints::Order, std::string>;
static OrderNames orderNames = boost::assign::list_of<OrderNames::relation>
	(Constraints::Order(Constraints::SortKey::None, Constraints::SortDirection::None), "None")
	(Constraints::Order(Constraints::SortKey::Chronology, Constraints::SortDirection::Ascending), "First")
	(Constraints::Order(Constraints::SortKey::Chronology, Constraints::SortDirection::Descending), "Last")
	(Constraints::Order(Constraints::SortKey::Size, Constraints::SortDirection::Ascending), "Shortest")
	(Constraints::Order(Constraints::SortKey::Size, Constraints::SortDirection::Descending), "Longest")
	(Constraints::Order(Constraints::SortKey::LBD, Constraints::SortDirection::Ascending), "LowestLBD")
	(Constraints::Order(Constraints::SortKey::LBD, Constraints::SortDirection::Descending), "HighestLBD")
	(Constraints::Order(Constraints::SortKey::TimeDegree, Constraints::SortDirection::Ascending), "LowestTimeDegree")
	(Constraints::Order(Constraints::SortKey::TimeDegree, Constraints::SortDirection::Descending), "HighestTimeDegree");

////////////////////////////////////////////////////////////////////////////////////////////////////

std::ostream &operator<<(std::ostream &ostream, const Constraints::Order &order)
{
	const auto match = orderNames.left.find(order);

	if (match == orderNames.left.end())
		return (ostream << "Unknown");

	return (ostream << (*match).second);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::istream &operator>>(std::istream &istream, Constraints::Order &order)
{
	std::string orderName;
	istream >> orderName;

	const auto match = orderNames.right.find(orderName);

	if (match == orderNames.right.end())
	{
		order.first = Constraints::SortKey::None;
		order.second = Constraints::SortDirection::None;
	}
	else
		order = (*match).second;

	return istream;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

}
}
