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

	CHECK(a.timeRange().min == 0);
	CHECK(a.timeRange().max == 0);
	CHECK(b.timeRange().min == 0);
	CHECK(b.timeRange().max == 2);
	CHECK(c.timeRange().min == 1);
	CHECK(c.timeRange().max == 100);
	CHECK(d.timeRange().min == 8);
	CHECK(d.timeRange().max == 100);
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
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

TEST_CASE("[constraints] Constraints are correctly generalized and subsumed", "[constraints]")
{
	const auto a = ginkgo::parseGroundConstraint({"holds(a, 0)", "holds(b, 0)", "holds(c, 0)"});
	const auto ga = ginkgo::GeneralizedConstraint(a);
	const auto b = ginkgo::parseGroundConstraint({"holds(a, 5)", "holds(b, 5)", "holds(c, 5)"});
	const auto gb = ginkgo::GeneralizedConstraint(b);
	const auto c = ginkgo::parseGroundConstraint({"holds(a, 4)", "holds(b, 5)", "holds(c, 7)"});
	const auto gc = ginkgo::GeneralizedConstraint(c);

	CHECK(ginkgo::subsumes(gb, a));
	CHECK(ginkgo::subsumes(ga, b));
	CHECK_FALSE(ginkgo::subsumes(ga, c));
	CHECK_FALSE(ginkgo::subsumes(gc, a));

	std::stringstream ob;
	ob << gb;

	CHECK(ob.str() == ":- holds(a,T), holds(b,T), holds(c,T), time(T).");

	const auto d = ginkgo::parseGroundConstraint({"apply(a, 6)", "holds(b, 6)", "holds(c, 6)"});
	const auto gd = ginkgo::GeneralizedConstraint(d);

	// Actions may not be applied in time step 0 → ensure shift by one
	std::stringstream od;
	od << gd;

	CHECK(od.str() == ":- apply(a,T+1), holds(b,T+1), holds(c,T+1), time(T), time(T+1).");
}

////////////////////////////////////////////////////////////////////////////////////////////////////

/*
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
*/

////////////////////////////////////////////////////////////////////////////////////////////////////

TEST_CASE("[constraints] Ground constraints are correctly subsumed by other ones", "[constraints]")
{
	const auto a = ginkgo::parseGroundConstraint({"holds(a, 0)", "holds(b, 0)", "holds(c, 0)", "holds(d, 0)", "holds(e, 0)"});
	const auto b = ginkgo::parseGroundConstraint({"holds(a, 0)", "holds(b, 0)", "holds(c, 0)", "holds(d, 0)"});
	const auto c = ginkgo::parseGroundConstraint({"holds(b, 0)", "holds(d, 0)"});
	const auto d = ginkgo::parseGroundConstraint({"holds(a, 0)", "holds(b, 0)", "holds(c, 0)", "holds(f, 0)"});
	const auto e = ginkgo::parseGroundConstraint({"holds(e, 0)", "holds(d, 0)", "holds(a, 0)", "holds(b, 0)", "holds(c, 0)"});

	CHECK(ginkgo::subsumes(b, a));
	CHECK_FALSE(ginkgo::subsumes(a, b));

	CHECK(ginkgo::subsumes(c, a));
	CHECK_FALSE(ginkgo::subsumes(a, c));

	CHECK_FALSE(ginkgo::subsumes(d, a));
	CHECK_FALSE(ginkgo::subsumes(a, d));

	CHECK(ginkgo::subsumes(e, a));
	CHECK(ginkgo::subsumes(a, e));

	CHECK(ginkgo::subsumes(c, b));
	CHECK_FALSE(ginkgo::subsumes(b, c));

	CHECK_FALSE(ginkgo::subsumes(d, b));
	CHECK_FALSE(ginkgo::subsumes(b, d));

	CHECK(ginkgo::subsumes(b, e));
	CHECK_FALSE(ginkgo::subsumes(e, b));

	CHECK_FALSE(ginkgo::subsumes(d, c));
	CHECK_FALSE(ginkgo::subsumes(c, d));

	CHECK_FALSE(ginkgo::subsumes(e, c));
	CHECK(ginkgo::subsumes(c, e));

	CHECK_FALSE(ginkgo::subsumes(e, d));
	CHECK_FALSE(ginkgo::subsumes(d, e));
}

////////////////////////////////////////////////////////////////////////////////////////////////////

TEST_CASE("[constraints] Constraint subsumption is negation-sensitive", "[constraints]")
{
	const auto a = ginkgo::parseGroundConstraint({"holds(a, 0)", "not holds(b, 0)", "holds(c, 0)", "holds(d, 0)", "holds(e, 0)"});
	const auto b = ginkgo::parseGroundConstraint({"holds(a, 0)", "not holds(b, 0)", "holds(c, 0)", "holds(d, 0)", "not holds(e, 0)"});
	const auto c = ginkgo::parseGroundConstraint({"holds(a, 0)", "not holds(b, 0)", "holds(c, 0)", "holds(d, 0)"});

	CHECK(ginkgo::subsumes(c, a));
	CHECK(ginkgo::subsumes(c, b));

	CHECK_FALSE(ginkgo::subsumes(a, c));
	CHECK_FALSE(ginkgo::subsumes(b, c));

	CHECK_FALSE(ginkgo::subsumes(b, a));
	CHECK_FALSE(ginkgo::subsumes(a, b));
}

////////////////////////////////////////////////////////////////////////////////////////////////////

TEST_CASE("[constraints] Ground constraints are correctly subsumed by generalized constraints", "[constraints]")
{
	const auto a = ginkgo::parseGroundConstraint({"holds(a, 0)", "holds(b, 0)", "holds(c, 0)"});
	const auto ga = ginkgo::GeneralizedConstraint(a);
	const auto b = ginkgo::parseGroundConstraint({"holds(a, 0)", "holds(b, 1)", "holds(c, 2)"});
	const auto gb = ginkgo::GeneralizedConstraint(b);
	const auto c = ginkgo::parseGroundConstraint({"holds(a, 0)", "holds(b, 0)", "holds(c, 0)", "apply(spam, 37)"});
	const auto d = ginkgo::parseGroundConstraint({"holds(a, 0)", "holds(b, 1)", "holds(c, 1)", "holds(spam, 37)"});
	const auto e = ginkgo::parseGroundConstraint({"holds(a, 1)", "holds(b, 1)", "holds(c, 1)", "holds(spam, 37)"});
	const auto f = ginkgo::parseGroundConstraint({"holds(a, 0)", "holds(b, 1)", "holds(c, 2)", "holds(spam, 37)"});
	const auto g = ginkgo::parseGroundConstraint({"holds(a, 23)", "holds(b, 24)", "holds(c, 25)", "holds(spam, 0)", "holds(spam, 37)"});
	const auto h = ginkgo::parseGroundConstraint({"holds(a, 24)", "holds(b, 24)", "holds(c, 24)", "holds(spam, 0)", "holds(spam, 37)"});
	const auto i = ginkgo::parseGroundConstraint({"holds(a, 37)", "holds(b, 37)", "holds(c, 37)", "holds(spam, 0)", "holds(spam, 37)"});
	const auto j = ginkgo::parseGroundConstraint({"holds(a, 35)", "holds(b, 36)", "holds(c, 37)", "holds(spam, 0)", "holds(spam, 37)"});
	const auto k = ginkgo::parseGroundConstraint({"holds(a, 60)", "holds(b, 60)", "holds(c, 60)", "holds(spam, 20)"});
	const auto l = ginkgo::parseGroundConstraint({"holds(a, 60)", "holds(b, 60)", "holds(c, 60)", "holds(spam, 60)"});
	const auto m = ginkgo::parseGroundConstraint({"holds(a, 60)", "holds(b, 60)", "holds(c, 60)", "holds(spam, 80)"});

	CHECK(ginkgo::subsumes(ga, c));
	CHECK_FALSE(ginkgo::subsumes(ga, d));
	CHECK(ginkgo::subsumes(ga, e));
	CHECK_FALSE(ginkgo::subsumes(ga, f));
	CHECK_FALSE(ginkgo::subsumes(ga, g));
	CHECK(ginkgo::subsumes(ga, h));
	CHECK(ginkgo::subsumes(ga, i));
	CHECK_FALSE(ginkgo::subsumes(ga, j));
	CHECK(ginkgo::subsumes(ga, k));
	CHECK(ginkgo::subsumes(ga, l));
	CHECK(ginkgo::subsumes(ga, m));

	CHECK_FALSE(ginkgo::subsumes(gb, c));
	CHECK_FALSE(ginkgo::subsumes(gb, d));
	CHECK_FALSE(ginkgo::subsumes(gb, e));
	CHECK(ginkgo::subsumes(gb, f));
	CHECK(ginkgo::subsumes(gb, g));
	CHECK_FALSE(ginkgo::subsumes(gb, h));
	CHECK_FALSE(ginkgo::subsumes(gb, i));
	CHECK(ginkgo::subsumes(gb, j));
	CHECK_FALSE(ginkgo::subsumes(gb, k));
	CHECK_FALSE(ginkgo::subsumes(gb, l));
	CHECK_FALSE(ginkgo::subsumes(gb, m));
}

////////////////////////////////////////////////////////////////////////////////////////////////////

TEST_CASE("[constraints] Generalized constraint subsumption works with actions", "[constraints]")
{
	const auto a = ginkgo::parseGroundConstraint({"apply(a, 1)", "holds(b, 2)", "holds(c, 4)"});
	const auto ga = ginkgo::GeneralizedConstraint(a);
	const auto b = ginkgo::parseGroundConstraint({"apply(a, 5)", "holds(b, 6)", "holds(c, 8)", "holds(spam, 37)"});
	const auto c = ginkgo::parseGroundConstraint({"apply(a, 34)", "holds(b, 35)", "holds(c, 37)", "holds(spam, 37)"});
	const auto d = ginkgo::parseGroundConstraint({"apply(a, 8)", "holds(b, 6)", "holds(c, 8)", "holds(spam, 37)"});

	CHECK(ginkgo::subsumes(ga, b));
	CHECK(ginkgo::subsumes(ga, c));
	CHECK_FALSE(ginkgo::subsumes(ga, d));
}
