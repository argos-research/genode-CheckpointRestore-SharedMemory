/*
 * \brief  Monitoring rm session creation/destruction
 * \author Denis Huber
 * \date   2016-10-06
 */

#ifndef _RTCR_RM_SESSION_INFO_H_
#define _RTCR_RM_SESSION_INFO_H_

/* Genode includes */
#include <util/list.h>

/* Rtcr includes */
#include "../intercept/rm_session.h"


namespace Rtcr {
	struct Rm_session_info;
}

/**
 * List element for managing Rm_session_components
 */
struct Rtcr::Rm_session_info : Genode::List<Rm_session_info>::Element
{
	/**
	 * Reference to the session object; encapsulates capability and object's state
	 */
	Rm_session_component &session;
	/**
	 * Arguments provided for creating the session object
	 */
	const char           *args;

	Rm_session_info(Rm_session_component &session, const char* args)
	:
		session (session),
		args    (args)
	{ }

	Rm_session_info *find_by_ptr(Rm_session_component *ptr)
	{
		if(ptr == &session)
			return this;
		Rm_session_info *info = next();
		return info ? info->find_by_ptr(ptr) : 0;
	}

	void print(Genode::Output &output) const
	{
		using Genode::Hex;

		Genode::print(output, "session ", session.cap(), " args=", args);
	}
};

#endif /* _RTCR_RM_SESSION_INFO_H_ */
