#ifndef __SOLVING__CONSTRAINT_H
#define __SOLVING__CONSTRAINT_H

#include <set>

#include <ginkgo/solving/Literal.h>
#include <ginkgo/utils/Range.h>

namespace ginkgo
{

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Constraint
//
////////////////////////////////////////////////////////////////////////////////////////////////////

Range<size_t> computeTimeRange(const Literals &literals);
bool subsumes(const Literals &lhs, const Literals &rhs);
bool subsumes(const Literals &lhs, const Literals &rhs, int lhsOffset, int rhsOffset);
bool contains(const Literals &literals, const char *predicateName);

////////////////////////////////////////////////////////////////////////////////////////////////////

}

#endif
