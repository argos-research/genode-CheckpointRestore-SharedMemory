/*
 * \brief  Restorer of Target_child
 * \author Denis Huber
 * \date   2016-10-26
 */

#ifndef _RTCR_RESTORER_H_
#define _RTCR_RESTORER_H_

#include "target_state.h"
#include "target_child.h"

namespace Rtcr {
	class Restorer;

	constexpr bool restorer_verbose_debug = true;
}

class Rtcr::Restorer
{
private:
	/**
	 * Enable log output for debugging
	 */
	static constexpr bool verbose_debug = restorer_verbose_debug;
	Target_child &_child;
	Target_state &_state;

	void _prepare_threads(Genode::List<Stored_thread_info> &state_infos, Cpu_session_component &child_obj);

public:
	Restorer(Target_child &child, Target_state &state);

	void restore();
};

#endif /* _RTCR_RESTORER_H_ */
