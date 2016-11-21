#ifndef __FEEDBACK_LOOP__PARTIAL_PROGRAMS_H
#define __FEEDBACK_LOOP__PARTIAL_PROGRAMS_H

namespace ginkgo
{
namespace feedbackLoop
{
namespace production
{

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// PartialPrograms
//
////////////////////////////////////////////////////////////////////////////////////////////////////

constexpr const auto HorizonProgram =
R"(
% Horizon
time(0..horizon).
)";

////////////////////////////////////////////////////////////////////////////////////////////////////

constexpr const auto DegreeProgram =
R"(
% Degree
time(0..degree).
)";

////////////////////////////////////////////////////////////////////////////////////////////////////

constexpr const auto InitialStateProgram =
R"(
% Establish initial state
holds(Variable, Value, 0) :- initialState(Variable, Value).
)";

////////////////////////////////////////////////////////////////////////////////////////////////////

constexpr const auto ActionProgram =
R"(
% Perform actions
1 {occurs(Action, T) : action(Action)} 1 :- time(T), T > 0.

% Check preconditions
:- occurs(Action, T), precondition(Action, Variable, Value), not holds(Variable, Value, T - 1), time(T), time(T - 1).

% Apply effects
caused(Variable, Value, T) :- occurs(Action, T), postcondition(Action, _, Variable, Value).
modified(Variable, T) :- caused(Variable, Value, T).

holds(Variable, Value, T) :- caused(Variable, Value, T), time(T).
holds(Variable, Value, T) :- holds(Variable, Value, T - 1), not modified(Variable, T), time(T), time(T - 1).

% Check that variables have unique values
:- variable(Variable), not 1 {holds(Variable, Value, T) : contains(Variable, Value)} 1, time(T).

% Check mutexes
:- mutexGroup(MutexGroup), not {holds(Variable, Value, T) : contains(MutexGroup, Variable, Value)} 1, time(T).
)";

////////////////////////////////////////////////////////////////////////////////////////////////////

constexpr const auto GoalProgram =
R"(
% Verify that goal is met
:- goal(Variable, Value), not holds(Variable, Value, horizon).
)";

////////////////////////////////////////////////////////////////////////////////////////////////////

constexpr const auto StateGeneratorProgram =
R"(
% Generate any possible initial state
{holds(Variable, Value, 0)} :- contains(Variable, Value).
)";

////////////////////////////////////////////////////////////////////////////////////////////////////

constexpr const auto FluentClosureProgram =
R"(
% Iteratively build the fluent closure using forward chaining
{holds(Variable, Value, 0)} :- fluentClosure(Variable, Value).
fluentClosure(Variable, Value) :- initialState(Variable, Value).
fluentClosure(Variable1, Value1) :- action(Action), fluentClosure(Variable2, Value2) : precondition(Action, Variable2, Value2), postcondition(Action, _, Variable1, Value1).
)";

////////////////////////////////////////////////////////////////////////////////////////////////////

}
}
}

#endif
