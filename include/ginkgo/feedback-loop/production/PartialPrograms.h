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

const auto MetaEncoding =
R"(
% Horizon
time(0..horizon).

% Establish initial state
holds(F, 0) :- init(F).

% Perform actions
1 {apply(A, T) : action(A)} 1 :- time(T), T > 0.

% Check preconditions
:- apply(A, T), demands(A, F, true), not holds(F, T - 1), time(T), time(T - 1).
:- apply(A, T), demands(A, F, false), holds(F, T - 1), time(T), time(T - 1).

% Apply effects
holds(F, T) :- apply(A, T), adds(A, F), action(A), time(T).
del(F, T) :- apply(A, T), deletes(A, F), action(A), time(T).
holds(F, T) :- holds(F, T - 1), not del(F, T), time(T), time(T - 1).

% Verify that goal is met
:- goal(F, true), not holds(F, horizon).
:- goal(F, false), holds(F, horizon).
)";

////////////////////////////////////////////////////////////////////////////////////////////////////

const auto StateGeneratorEncoding =
R"(
% Generate any possible initial state
{holds(F, 0)} :- fluent(F).
)";

////////////////////////////////////////////////////////////////////////////////////////////////////

const auto FluentClosureEncoding =
R"(
% Iteratively build the fluent closure using forward chaining
{holds(F, 0)} :- fluentClosure(F).
fluentClosure(F) :- init(F).
fluentClosure(F1) :- action(A), fluentClosure(F2) : demands(A, F2, true), adds(A, F1).
)";

////////////////////////////////////////////////////////////////////////////////////////////////////

}
}
}

#endif
