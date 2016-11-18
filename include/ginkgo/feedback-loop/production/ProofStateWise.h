#ifndef __FEEDBACK_LOOP__PROOF_STATE_WISE_H
#define __FEEDBACK_LOOP__PROOF_STATE_WISE_H

#include <iostream>

#include <ginkgo/feedback-loop/production/Configuration.h>
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
// ProofStateWise
//
////////////////////////////////////////////////////////////////////////////////////////////////////

ProofResult testCandidateStateWise(const GeneralizedConstraint &candidate, std::istream &program,
	const Configuration<Plain> &configuration);

////////////////////////////////////////////////////////////////////////////////////////////////////

}
}
}

#endif
