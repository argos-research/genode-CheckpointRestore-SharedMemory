/*
 * \brief  Monitoring timer session creation/destruction
 * \author Denis Huber
 * \date   2016-10-07
 */

#ifndef _RTCR_TIMER_SESSION_INFO_COMPONENT_H_
#define _RTCR_TIMER_SESSION_INFO_COMPONENT_H_

/* Genode includes */
#include <util/list.h>

namespace Rtcr {
	struct Timer_session_info;

	// Forward declaration
	struct Timer_session_component;
}


/**
 * List element for monitoring session objects.
 * Each new connection from client to server is monitored here.
 */
struct Rtcr::Timer_session_info : Genode::List<Timer_session_info>::Element
{
	/**
	 * Reference to the session object; encapsulates capability and object's state
	 */
	Timer_session_component &session;
	/**
	 * Arguments provided for creating the session object
	 */
	const char *args;

	Timer_session_info(Timer_session_component &comp, const char* args)
	:
		session(comp),
		args(args)
	{ }

	Timer_session_info *find_by_ptr(Timer_session_component *ptr)
	{
		if(ptr == &session)
			return this;
		Timer_session_info *info = next();
		return info ? info->find_by_ptr(ptr) : 0;
	}
};

#endif /* _RTCR_TIMER_SESSION_INFO_COMPONENT_H_ */
