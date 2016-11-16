#include <catch.hpp>

#include <ginkgo/solving/GeneralizedConstraint.h>
#include <ginkgo/solving/GroundConstraint.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

/*
TEST_CASE("[constraints] Duplicate literals in constraints are eliminated", "[constraints]")
{
	ginkgo::deprecated::SymbolTable symbolTable;
	ginkgo::deprecated::Constraint a(0, ":- a, b, b, a, b, a(b(c(d(e)))), a(b(c(d(e)))), a(b(c(d(e)))).", symbolTable);
	ginkgo::deprecated::Constraint b(0, ":- a, b, a(b(c(d(e)))).", symbolTable);

	REQUIRE(a.literals().size() == 3);
	REQUIRE(b.literals().size() == 3);
}
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

TEST_CASE("[constraints] The time arguments of temporal literals are parsed correctly", "[constraints]")
{
	const auto a = ginkgo::parseGroundConstraint({"holds(a, 0)", "holds(b, 0)", "holds(c, 0)"});
	const auto b = ginkgo::parseGroundConstraint({"holds(a, 0)", "holds(b, 1)", "holds(c, 2)"});
	const auto c = ginkgo::parseGroundConstraint({"holds(a, 100)", "holds(b, 1)", "holds(c, 50)"});
	const auto d = ginkgo::parseGroundConstraint({"holds(a, 12)", "holds(b, 8)", "holds(c, 100)"});

	REQUIRE(a.timeRange().min == 0);
	REQUIRE(a.timeRange().max == 0);
	REQUIRE(b.timeRange().min == 0);
	REQUIRE(b.timeRange().max == 2);
	REQUIRE(c.timeRange().min == 1);
	REQUIRE(c.timeRange().max == 100);
	REQUIRE(d.timeRange().min == 8);
	REQUIRE(d.timeRange().max == 100);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

/*
TEST_CASE("[constraints] Literal identifiers are collected correctly", "[constraints]")
{
	ginkgo::deprecated::SymbolTable symbolTable;
	ginkgo::deprecated::Constraint a(0, ":- a, b, c, terminal, holds(f, 7), not apply(t, 1).", symbolTable);

	REQUIRE(a.containsIdentifier("a"));
	REQUIRE(a.containsIdentifier("terminal"));
	REQUIRE(a.containsIdentifier("holds"));
	REQUIRE_FALSE(a.containsIdentifier("e"));
	REQUIRE_FALSE(a.containsIdentifier("f"));
	REQUIRE_FALSE(a.containsIdentifier("7"));
	REQUIRE(a.containsIdentifier("apply"));
}

////////////////////////////////////////////////////////////////////////////////////////////////////

TEST_CASE("[constraints] Constraints are correctly generalized and subsumed", "[constraints]")
{
	ginkgo::deprecated::SymbolTable symbolTable;
	auto a = std::make_shared<ginkgo::deprecated::Constraint>(0, ":- holds(a, 0), holds(b, 0), holds(c, 0).", symbolTable);
	auto b = std::make_shared<ginkgo::deprecated::Constraint>(0, ":- holds(a, 5), holds(b, 5), holds(c, 5).", symbolTable);
	auto c = std::make_shared<ginkgo::deprecated::Constraint>(0, ":- holds(a, 4), holds(b, 5), holds(c, 7).", symbolTable);

	REQUIRE(ginkgo::deprecated::GeneralizedConstraint(b).subsumes(*a));
	REQUIRE(ginkgo::deprecated::GeneralizedConstraint(a).subsumes(*b));
	REQUIRE_FALSE(ginkgo::deprecated::GeneralizedConstraint(a).subsumes(*c));
	REQUIRE_FALSE(ginkgo::deprecated::GeneralizedConstraint(c).subsumes(*a));

	std::stringstream outputB;
	ginkgo::deprecated::GeneralizedConstraint(a).print(outputB);

	REQUIRE(outputB.str() == ":- time(T), holds(a, T), holds(b, T), holds(c, T).");

	auto d = std::make_shared<ginkgo::deprecated::Constraint>(0, ":- apply(a, 6), holds(b, 6), holds(c, 6).", symbolTable);

	// Actions may not be applied in time step 0 → ensure shift by one
	std::stringstream outputD;
	ginkgo::deprecated::GeneralizedConstraint(d).print(outputD);

	REQUIRE(outputD.str() == ":- time(T), time(T+1), apply(a, T+1), holds(b, T+1), holds(c, T+1).");
}

////////////////////////////////////////////////////////////////////////////////////////////////////

TEST_CASE("[constraints] Literals are eliminated as specified", "[constraints]")
{
	ginkgo::deprecated::SymbolTable symbolTable;
	ginkgo::deprecated::Constraint a(0, ":- a, b, c, d, e.", symbolTable);
	ginkgo::deprecated::Constraint b(0, ":- a, b, c, e.", symbolTable);
	ginkgo::deprecated::Constraint c(0, ":- b, c, e.", symbolTable);

	REQUIRE(a.withoutLiterals(3)->subsumes(b));
	REQUIRE(b.subsumes(*a.withoutLiterals(3)));

	REQUIRE(b.withoutLiterals(0)->subsumes(c));
	REQUIRE(c.subsumes(*b.withoutLiterals(0)));
}

////////////////////////////////////////////////////////////////////////////////////////////////////

TEST_CASE("[constraints] Constraits subsume themselves", "[constraints]")
{
	ginkgo::deprecated::SymbolTable symbolTable;
	ginkgo::deprecated::Constraint a(0, ":- a, b, c, d, e.", symbolTable);
	ginkgo::deprecated::Constraint b(0, ":- a, b, not c, d, not e.", symbolTable);

	REQUIRE(a.subsumes(a));
	REQUIRE(b.subsumes(b));
}

////////////////////////////////////////////////////////////////////////////////////////////////////

TEST_CASE("[constraints] Constraints are correctly subsumed by other constraints", "[constraints]")
{
	ginkgo::deprecated::SymbolTable symbolTable;
	ginkgo::deprecated::Constraint a(0, ":- a, b, c, d, e.", symbolTable);
	ginkgo::deprecated::Constraint b(0, ":- a, b, c, d.", symbolTable);
	ginkgo::deprecated::Constraint c(0, ":- b, d.", symbolTable);
	ginkgo::deprecated::Constraint d(0, ":- a, b, c, f.", symbolTable);
	ginkgo::deprecated::Constraint e(0, ":- e, d, a, b, c.", symbolTable);

	REQUIRE(b.subsumes(a));
	REQUIRE_FALSE(a.subsumes(b));

	REQUIRE(c.subsumes(a));
	REQUIRE_FALSE(a.subsumes(c));

	REQUIRE_FALSE(d.subsumes(a));
	REQUIRE_FALSE(a.subsumes(d));

	REQUIRE(e.subsumes(a));
	REQUIRE(a.subsumes(e));

	REQUIRE(c.subsumes(b));
	REQUIRE_FALSE(b.subsumes(c));

	REQUIRE_FALSE(d.subsumes(b));
	REQUIRE_FALSE(b.subsumes(d));

	REQUIRE(b.subsumes(e));
	REQUIRE_FALSE(e.subsumes(b));

	REQUIRE_FALSE(d.subsumes(c));
	REQUIRE_FALSE(c.subsumes(d));

	REQUIRE_FALSE(e.subsumes(c));
	REQUIRE(c.subsumes(e));

	REQUIRE_FALSE(e.subsumes(d));
	REQUIRE_FALSE(d.subsumes(e));
}

////////////////////////////////////////////////////////////////////////////////////////////////////

TEST_CASE("[constraints] Constraint subsumption is negation-sensitive", "[constraints]")
{
	ginkgo::deprecated::SymbolTable symbolTable;
	ginkgo::deprecated::Constraint a(0, ":- a, not b, c, d, e.", symbolTable);
	ginkgo::deprecated::Constraint b(0, ":- a, not b, c, d, not e.", symbolTable);
	ginkgo::deprecated::Constraint c(0, ":- a, not b, c, d.", symbolTable);

	REQUIRE(c.subsumes(a));
	REQUIRE(c.subsumes(b));

	REQUIRE_FALSE(a.subsumes(c));
	REQUIRE_FALSE(b.subsumes(c));

	REQUIRE_FALSE(b.subsumes(a));
	REQUIRE_FALSE(a.subsumes(b));
}

////////////////////////////////////////////////////////////////////////////////////////////////////

TEST_CASE("[constraints] Generalized constraints are correctly subsumed by other generalized constraints", "[constraints]")
{
	ginkgo::deprecated::SymbolTable symbolTable;

	auto a = std::make_shared<ginkgo::deprecated::Constraint>(0, ":- holds(a, 0), holds(b, 0), holds(c, 0).", symbolTable);
	auto b = std::make_shared<ginkgo::deprecated::Constraint>(0, ":- holds(a, 0), holds(b, 1), holds(c, 2).", symbolTable);

	auto generalizedA = ginkgo::deprecated::GeneralizedConstraint(a);
	auto generalizedB = ginkgo::deprecated::GeneralizedConstraint(b);

	ginkgo::deprecated::Constraint c(0, ":- holds(a, 0), holds(b, 0), holds(c, 0), holds(spam, 37).", symbolTable);
	ginkgo::deprecated::Constraint d(0, ":- holds(a, 0), holds(b, 1), holds(c, 1), holds(spam, 37).", symbolTable);
	ginkgo::deprecated::Constraint e(0, ":- holds(a, 1), holds(b, 1), holds(c, 1), holds(spam, 37).", symbolTable);
	ginkgo::deprecated::Constraint f(0, ":- holds(a, 0), holds(b, 1), holds(c, 2), holds(spam, 37).", symbolTable);
	ginkgo::deprecated::Constraint g(0, ":- holds(a, 23), holds(b, 24), holds(c, 25), holds(spam, 0), holds(spam, 37).", symbolTable);
	ginkgo::deprecated::Constraint h(0, ":- holds(a, 24), holds(b, 24), holds(c, 24), holds(spam, 0), holds(spam, 37).", symbolTable);
	ginkgo::deprecated::Constraint i(0, ":- holds(a, 37), holds(b, 37), holds(c, 37), holds(spam, 0), holds(spam, 37).", symbolTable);
	ginkgo::deprecated::Constraint j(0, ":- holds(a, 35), holds(b, 36), holds(c, 37), holds(spam, 0), holds(spam, 37).", symbolTable);
	ginkgo::deprecated::Constraint k(0, ":- holds(a, 60), holds(b, 60), holds(c, 60), holds(spam, 20).", symbolTable);
	ginkgo::deprecated::Constraint l(0, ":- holds(a, 60), holds(b, 60), holds(c, 60), holds(spam, 60).", symbolTable);
	ginkgo::deprecated::Constraint m(0, ":- holds(a, 60), holds(b, 60), holds(c, 60), holds(spam, 80).", symbolTable);

	REQUIRE(generalizedA.subsumes(c));
	REQUIRE_FALSE(generalizedA.subsumes(d));
	REQUIRE(generalizedA.subsumes(e));
	REQUIRE_FALSE(generalizedA.subsumes(f));
	REQUIRE_FALSE(generalizedA.subsumes(g));
	REQUIRE(generalizedA.subsumes(h));
	REQUIRE(generalizedA.subsumes(i));
	REQUIRE_FALSE(generalizedA.subsumes(j));
	REQUIRE(generalizedA.subsumes(k));
	REQUIRE(generalizedA.subsumes(l));
	REQUIRE(generalizedA.subsumes(m));

	REQUIRE_FALSE(generalizedB.subsumes(c));
	REQUIRE_FALSE(generalizedB.subsumes(d));
	REQUIRE_FALSE(generalizedB.subsumes(e));
	REQUIRE(generalizedB.subsumes(f));
	REQUIRE(generalizedB.subsumes(g));
	REQUIRE_FALSE(generalizedB.subsumes(h));
	REQUIRE_FALSE(generalizedB.subsumes(i));
	REQUIRE(generalizedB.subsumes(j));
	REQUIRE_FALSE(generalizedB.subsumes(k));
	REQUIRE_FALSE(generalizedB.subsumes(l));
	REQUIRE_FALSE(generalizedB.subsumes(m));
}

////////////////////////////////////////////////////////////////////////////////////////////////////

TEST_CASE("[constraints] Generalized constraint subsumption works with actions", "[constraints]")
{
	ginkgo::deprecated::SymbolTable symbolTable;

	auto a = std::make_shared<ginkgo::deprecated::Constraint>(0, ":- apply(a, 1), holds(b, 2), holds(c, 4).", symbolTable);

	auto generalizedA = ginkgo::deprecated::GeneralizedConstraint(a);

	ginkgo::deprecated::Constraint b(0, ":- apply(a, 5), holds(b, 6), holds(c, 8), holds(spam, 37).", symbolTable);
	ginkgo::deprecated::Constraint c(0, ":- apply(a, 34), holds(b, 35), holds(c, 37), holds(spam, 37).", symbolTable);
	ginkgo::deprecated::Constraint d(0, ":- apply(a, 8), holds(b, 6), holds(c, 8), holds(spam, 37).", symbolTable);

	REQUIRE(generalizedA.subsumes(b));
	REQUIRE(generalizedA.subsumes(c));
	REQUIRE_FALSE(generalizedA.subsumes(d));
}

*/
