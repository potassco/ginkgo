#include <gtest/gtest.h>

#include <ginkgo/solving/Constraint.h>
#include <ginkgo/solving/GeneralizedConstraint.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

TEST(ConstraintTests, Repetition)
{
	ginkgo::SymbolTable symbolTable;
	ginkgo::Constraint a(0, ":- a, b, b, a, b, a(b(c(d(e)))), a(b(c(d(e)))), a(b(c(d(e)))).", symbolTable);
	ginkgo::Constraint b(0, ":- a, b, a(b(c(d(e)))).", symbolTable);

	EXPECT_EQ(a.literals().size(), 3);
	EXPECT_EQ(b.literals().size(), 3);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

TEST(ConstraintTests, TimeRange)
{
	ginkgo::SymbolTable symbolTable;
	ginkgo::Constraint a(0, ":- holds(a, 0), holds(b, 0), holds(c, 0).", symbolTable);
	ginkgo::Constraint b(0, ":- holds(a, 0), holds(b, 1), holds(c, 2).", symbolTable);
	ginkgo::Constraint c(0, ":- holds(a, 100), holds(b, 1), holds(c, 50).", symbolTable);
	ginkgo::Constraint d(0, ":- holds(a, 12), holds(b, 8), holds(c, 100).", symbolTable);

	EXPECT_EQ(a.timeRange(), std::make_tuple(0, 0));
	EXPECT_EQ(b.timeRange(), std::make_tuple(0, 2));
	EXPECT_EQ(c.timeRange(), std::make_tuple(1, 100));
	EXPECT_EQ(d.timeRange(), std::make_tuple(8, 100));
}

////////////////////////////////////////////////////////////////////////////////////////////////////

TEST(ConstraintTests, ContainsIdentifier)
{
	ginkgo::SymbolTable symbolTable;
	ginkgo::Constraint a(0, ":- a, b, c, terminal, holds(f, 7), not apply(t, 1).", symbolTable);

	EXPECT_TRUE(a.containsIdentifier("a"));
	EXPECT_TRUE(a.containsIdentifier("terminal"));
	EXPECT_TRUE(a.containsIdentifier("holds"));
	EXPECT_FALSE(a.containsIdentifier("e"));
	EXPECT_FALSE(a.containsIdentifier("f"));
	EXPECT_FALSE(a.containsIdentifier("7"));
	EXPECT_TRUE(a.containsIdentifier("apply"));
}

////////////////////////////////////////////////////////////////////////////////////////////////////

TEST(ConstraintTests, Generalizing)
{
	ginkgo::SymbolTable symbolTable;
	auto a = std::make_shared<ginkgo::Constraint>(0, ":- holds(a, 0), holds(b, 0), holds(c, 0).", symbolTable);
	auto b = std::make_shared<ginkgo::Constraint>(0, ":- holds(a, 5), holds(b, 5), holds(c, 5).", symbolTable);
	auto c = std::make_shared<ginkgo::Constraint>(0, ":- holds(a, 4), holds(b, 5), holds(c, 7).", symbolTable);

	EXPECT_TRUE(ginkgo::GeneralizedConstraint(b).subsumes(*a));
	EXPECT_TRUE(ginkgo::GeneralizedConstraint(a).subsumes(*b));
	EXPECT_FALSE(ginkgo::GeneralizedConstraint(a).subsumes(*c));
	EXPECT_FALSE(ginkgo::GeneralizedConstraint(c).subsumes(*a));

	std::stringstream outputB;
	ginkgo::GeneralizedConstraint(a).print(outputB);

	EXPECT_EQ(outputB.str(), ":- time(T), holds(a, T), holds(b, T), holds(c, T).");

	auto d = std::make_shared<ginkgo::Constraint>(0, ":- apply(a, 6), holds(b, 6), holds(c, 6).", symbolTable);

	// Actions may not be applied in time step 0 → ensure shift by one
	std::stringstream outputD;
	ginkgo::GeneralizedConstraint(d).print(outputD);

	EXPECT_EQ(outputD.str(), ":- time(T), time(T+1), apply(a, T+1), holds(b, T+1), holds(c, T+1).");
}

////////////////////////////////////////////////////////////////////////////////////////////////////

TEST(ConstraintTests, LiteralElimination)
{
	ginkgo::SymbolTable symbolTable;
	ginkgo::Constraint a(0, ":- a, b, c, d, e.", symbolTable);
	ginkgo::Constraint b(0, ":- a, b, c, e.", symbolTable);
	ginkgo::Constraint c(0, ":- b, c, e.", symbolTable);

	EXPECT_TRUE(a.withoutLiterals(3)->subsumes(b));
	EXPECT_TRUE(b.subsumes(*a.withoutLiterals(3)));

	EXPECT_TRUE(b.withoutLiterals(0)->subsumes(c));
	EXPECT_TRUE(c.subsumes(*b.withoutLiterals(0)));
}

////////////////////////////////////////////////////////////////////////////////////////////////////

TEST(ConstraintTests, SubsumptionIdentity)
{
	ginkgo::SymbolTable symbolTable;
	ginkgo::Constraint a(0, ":- a, b, c, d, e.", symbolTable);
	ginkgo::Constraint b(0, ":- a, b, not c, d, not e.", symbolTable);

	EXPECT_TRUE(a.subsumes(a));
	EXPECT_TRUE(b.subsumes(b));
}

////////////////////////////////////////////////////////////////////////////////////////////////////

TEST(ConstraintTests, SubsumptionSimple)
{
	ginkgo::SymbolTable symbolTable;
	ginkgo::Constraint a(0, ":- a, b, c, d, e.", symbolTable);
	ginkgo::Constraint b(0, ":- a, b, c, d.", symbolTable);
	ginkgo::Constraint c(0, ":- b, d.", symbolTable);
	ginkgo::Constraint d(0, ":- a, b, c, f.", symbolTable);
	ginkgo::Constraint e(0, ":- e, d, a, b, c.", symbolTable);

	EXPECT_TRUE(b.subsumes(a));
	EXPECT_FALSE(a.subsumes(b));

	EXPECT_TRUE(c.subsumes(a));
	EXPECT_FALSE(a.subsumes(c));

	EXPECT_FALSE(d.subsumes(a));
	EXPECT_FALSE(a.subsumes(d));

	EXPECT_TRUE(e.subsumes(a));
	EXPECT_TRUE(a.subsumes(e));

	EXPECT_TRUE(c.subsumes(b));
	EXPECT_FALSE(b.subsumes(c));

	EXPECT_FALSE(d.subsumes(b));
	EXPECT_FALSE(b.subsumes(d));

	EXPECT_TRUE(b.subsumes(e));
	EXPECT_FALSE(e.subsumes(b));

	EXPECT_FALSE(d.subsumes(c));
	EXPECT_FALSE(c.subsumes(d));

	EXPECT_FALSE(e.subsumes(c));
	EXPECT_TRUE(c.subsumes(e));

	EXPECT_FALSE(e.subsumes(d));
	EXPECT_FALSE(d.subsumes(e));
}

////////////////////////////////////////////////////////////////////////////////////////////////////

TEST(ConstraintTests, Negation)
{
	ginkgo::SymbolTable symbolTable;
	ginkgo::Constraint a(0, ":- a, not b, c, d, e.", symbolTable);
	ginkgo::Constraint b(0, ":- a, not b, c, d, not e.", symbolTable);
	ginkgo::Constraint c(0, ":- a, not b, c, d.", symbolTable);

	EXPECT_TRUE(c.subsumes(a));
	EXPECT_TRUE(c.subsumes(b));

	EXPECT_FALSE(a.subsumes(c));
	EXPECT_FALSE(b.subsumes(c));

	EXPECT_FALSE(b.subsumes(a));
	EXPECT_FALSE(a.subsumes(b));
}

////////////////////////////////////////////////////////////////////////////////////////////////////

TEST(ConstraintTests, SubsumptionGeneralized)
{
	ginkgo::SymbolTable symbolTable;

	auto a = std::make_shared<ginkgo::Constraint>(0, ":- holds(a, 0), holds(b, 0), holds(c, 0).", symbolTable);
	auto b = std::make_shared<ginkgo::Constraint>(0, ":- holds(a, 0), holds(b, 1), holds(c, 2).", symbolTable);

	auto generalizedA = ginkgo::GeneralizedConstraint(a);
	auto generalizedB = ginkgo::GeneralizedConstraint(b);

	ginkgo::Constraint c(0, ":- holds(a, 0), holds(b, 0), holds(c, 0), holds(spam, 37).", symbolTable);
	ginkgo::Constraint d(0, ":- holds(a, 0), holds(b, 1), holds(c, 1), holds(spam, 37).", symbolTable);
	ginkgo::Constraint e(0, ":- holds(a, 1), holds(b, 1), holds(c, 1), holds(spam, 37).", symbolTable);
	ginkgo::Constraint f(0, ":- holds(a, 0), holds(b, 1), holds(c, 2), holds(spam, 37).", symbolTable);
	ginkgo::Constraint g(0, ":- holds(a, 23), holds(b, 24), holds(c, 25), holds(spam, 0), holds(spam, 37).", symbolTable);
	ginkgo::Constraint h(0, ":- holds(a, 24), holds(b, 24), holds(c, 24), holds(spam, 0), holds(spam, 37).", symbolTable);
	ginkgo::Constraint i(0, ":- holds(a, 37), holds(b, 37), holds(c, 37), holds(spam, 0), holds(spam, 37).", symbolTable);
	ginkgo::Constraint j(0, ":- holds(a, 35), holds(b, 36), holds(c, 37), holds(spam, 0), holds(spam, 37).", symbolTable);
	ginkgo::Constraint k(0, ":- holds(a, 60), holds(b, 60), holds(c, 60), holds(spam, 20).", symbolTable);
	ginkgo::Constraint l(0, ":- holds(a, 60), holds(b, 60), holds(c, 60), holds(spam, 60).", symbolTable);
	ginkgo::Constraint m(0, ":- holds(a, 60), holds(b, 60), holds(c, 60), holds(spam, 80).", symbolTable);

	EXPECT_TRUE(generalizedA.subsumes(c));
	EXPECT_FALSE(generalizedA.subsumes(d));
	EXPECT_TRUE(generalizedA.subsumes(e));
	EXPECT_FALSE(generalizedA.subsumes(f));
	EXPECT_FALSE(generalizedA.subsumes(g));
	EXPECT_TRUE(generalizedA.subsumes(h));
	EXPECT_TRUE(generalizedA.subsumes(i));
	EXPECT_FALSE(generalizedA.subsumes(j));
	EXPECT_TRUE(generalizedA.subsumes(k));
	EXPECT_TRUE(generalizedA.subsumes(l));
	EXPECT_TRUE(generalizedA.subsumes(m));

	EXPECT_FALSE(generalizedB.subsumes(c));
	EXPECT_FALSE(generalizedB.subsumes(d));
	EXPECT_FALSE(generalizedB.subsumes(e));
	EXPECT_TRUE(generalizedB.subsumes(f));
	EXPECT_TRUE(generalizedB.subsumes(g));
	EXPECT_FALSE(generalizedB.subsumes(h));
	EXPECT_FALSE(generalizedB.subsumes(i));
	EXPECT_TRUE(generalizedB.subsumes(j));
	EXPECT_FALSE(generalizedB.subsumes(k));
	EXPECT_FALSE(generalizedB.subsumes(l));
	EXPECT_FALSE(generalizedB.subsumes(m));
}

////////////////////////////////////////////////////////////////////////////////////////////////////

TEST(ConstraintTests, SubsumptionGeneralizedWithActions)
{
	ginkgo::SymbolTable symbolTable;

	auto a = std::make_shared<ginkgo::Constraint>(0, ":- apply(a, 1), holds(b, 2), holds(c, 4).", symbolTable);

	auto generalizedA = ginkgo::GeneralizedConstraint(a);

	ginkgo::Constraint b(0, ":- apply(a, 5), holds(b, 6), holds(c, 8), holds(spam, 37).", symbolTable);
	ginkgo::Constraint c(0, ":- apply(a, 34), holds(b, 35), holds(c, 37), holds(spam, 37).", symbolTable);
	ginkgo::Constraint d(0, ":- apply(a, 8), holds(b, 6), holds(c, 8), holds(spam, 37).", symbolTable);

	EXPECT_TRUE(generalizedA.subsumes(b));
	EXPECT_TRUE(generalizedA.subsumes(c));
	EXPECT_FALSE(generalizedA.subsumes(d));
}
