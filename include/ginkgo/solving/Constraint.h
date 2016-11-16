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

enum class ConstraintOutputFormat
{
	Normal,
	Generalized
};

////////////////////////////////////////////////////////////////////////////////////////////////////

Range<size_t> computeTimeRange(const Literals &literals);

////////////////////////////////////////////////////////////////////////////////////////////////////

}

#endif
