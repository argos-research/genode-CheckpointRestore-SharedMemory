/*
 * \brief  Monitoring rm session creation/destruction
 * \author Denis Huber
 * \date   2016-10-06
 */

#ifndef _RTCR_RM_SESSION_INFO_COMPONENT_H_
#define _RTCR_RM_SESSION_INFO_COMPONENT_H_

/* Genode includes */
#include <util/list.h>

/* Rtcr includes */
//#include "../intercept/rm_session.h"

namespace Rtcr {
	struct Rm_session_info;

	// Forward declaration
	struct Rm_session_component;
}

/**
 * List element for managing Rm_session_components
 */
struct Rtcr::Rm_session_info : Genode::List<Rm_session_info>::Element
{
	/**
	 * Reference to the session object; encapsulates capability and object's state
	 */
	Rm_session_component &rms;
	/**
	 * Arguments provided for creating the session object
	 */
	const char           *args;

	Rm_session_info(Rm_session_component &rms, const char* args)
	:
		rms  (rms),
		args (args)
	{ }

	Rm_session_info *find_by_ptr(Rm_session_component *ptr)
	{
		if(ptr == &rms)
			return this;
		Rm_session_info *rms_info = next();
		return rms_info ? rms_info->find_by_ptr(ptr) : 0;
	}
};

#endif /* _RTCR_RM_SESSION_INFO_COMPONENT_H_ */
