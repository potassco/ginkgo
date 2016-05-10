#ifndef __SOLVING__CLASP_OUTPUT_PARSING_H
#define __SOLVING__CLASP_OUTPUT_PARSING_H

#include <ginkgo/solving/Process.h>
#include <ginkgo/solving/Satisfiability.h>

namespace ginkgo
{

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// ClaspOutputParsing
//
////////////////////////////////////////////////////////////////////////////////////////////////////

float parseForSolvingTime(std::stringstream &claspOutput);
Satisfiability parseForSatisfiability(std::stringstream &claspOutputJson);
bool parseForErrors(std::stringstream &claspOutput);
bool parseForWarnings(std::stringstream &claspOutput);

////////////////////////////////////////////////////////////////////////////////////////////////////

}

#endif
