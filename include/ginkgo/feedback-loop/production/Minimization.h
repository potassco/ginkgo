#ifndef __FEEDBACK_LOOP__MINIMIZATION_H
#define __FEEDBACK_LOOP__MINIMIZATION_H

#include <iostream>

#include <ginkgo/feedback-loop/production/Configuration.h>
#include <ginkgo/feedback-loop/production/Environment.h>
#include <ginkgo/feedback-loop/production/ProofResult.h>
#include <ginkgo/solving/GeneralizedConstraint.h>

namespace ginkgo
{
namespace feedbackLoop
{
namespace production
{

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Minimization
//
////////////////////////////////////////////////////////////////////////////////////////////////////

GeneralizedConstraint minimizeConstraint(const GeneralizedConstraint &provenConstraint,
	size_t linearIncrement, std::istream &program, const Configuration<Plain> &configuration,
	const Environment &environment);

ProofResult testCandidateStateWise();

////////////////////////////////////////////////////////////////////////////////////////////////////

}
}
}

#endif
