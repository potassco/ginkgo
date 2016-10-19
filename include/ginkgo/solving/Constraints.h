#ifndef __SOLVING__CONSTRAINTS_H
#define __SOLVING__CONSTRAINTS_H

#include <set>

#include <ginkgo/solving/Constraint.h>
#include <ginkgo/solving/GeneralizedConstraint.h>

namespace ginkgo
{
namespace deprecated
{

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Constraints
//
////////////////////////////////////////////////////////////////////////////////////////////////////

class Constraints
{
	public:
		enum class SortKey
		{
			None,
			Chronology,
			Size,
			LBD,
			TimeDegree
		};

		enum class SortDirection
		{
			None,
			Ascending,
			Descending
		};

		using Order = std::pair<SortKey, SortDirection>;
		using Orders = std::set<Order>;

		using DataType = std::vector<ConstraintPtr>;
		using iterator = DataType::iterator;
		using const_iterator = DataType::const_iterator;
		using reverse_iterator = DataType::reverse_iterator;
		using reference = DataType::reference;
		using value_type = DataType::value_type;

	public:
		Constraints(SymbolTable &symbolTable);
		Constraints(SymbolTable &symbolTable, const std::string &string);

		void sortBy(SortKey sortKey, SortDirection sortDirection = SortDirection::Ascending, bool force = true);
		void select(Order order, size_t number, Constraints &subset, size_t &beginIndex);

		float averageNumberOfLiterals() const;

		void removeConstraintsSubsumedBy(const Constraint &constraint);
		void removeConstraintsSubsumedBy(const GeneralizedConstraint &generalizedConstraint);
		void removeConstraintsContainingIdentifier(const std::string &identifier);
		void removeConstraintsWithTooHighDegree(size_t maxDegree);
		void removeConstraintsContainingTooManyLiterals(size_t maxNumberOfLiterals);
		void removeLiterals(const std::string &literalName);
		bool subsumes(const Constraint &constraint) const;

		size_t size() const;
		void clear();
		bool empty() const;
		iterator begin();
		iterator end();
		const_iterator cbegin() const;
		const_iterator cend() const;
		reverse_iterator rbegin();
		reverse_iterator rend();
		reference back();
		void pop_back();
		void push_back(const DataType::value_type &x);
		iterator erase(iterator first, iterator last);
		iterator erase(iterator position);

	private:
		Constraints(const Constraints &other) = delete;

		DataType m_constraints;

		SortKey m_currentSortKey;
		SortDirection m_currentSortDirection;

		SymbolTable &m_symbolTable;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

std::ostream &operator<<(std::ostream &ostream, const Constraints &constraints);
std::ostream &operator<<(std::ostream &ostream, const Constraints::Order &order);
std::istream &operator>>(std::istream &istream, Constraints::Order &order);

////////////////////////////////////////////////////////////////////////////////////////////////////

}
}

#endif
