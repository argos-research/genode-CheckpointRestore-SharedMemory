/*
 * \brief  Restorer of Target_child
 * \author Denis Huber
 * \date   2016-10-26
 */

#ifndef _RTCR_RESTORER_H_
#define _RTCR_RESTORER_H_

#include "target_state.h"

namespace Rtcr {
	class Restorer;

	// Forward declaration
	class Target_child;
}

class Rtcr::Restorer
{
private:
	Target_child &_child;
	Target_state &_state;

public:
	Restorer(Target_child &child, Target_state &state);

	void restore();
};

#endif /* _RTCR_RESTORER_H_ */
