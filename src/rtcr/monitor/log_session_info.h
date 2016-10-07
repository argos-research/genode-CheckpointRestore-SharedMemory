/*
 * \brief  Monitoring log session creation
 * \author Denis Huber
 * \date   2016-10-06
 */

#ifndef _RTCR_LOG_SESSION_INFO_COMPONENT_H_
#define _RTCR_LOG_SESSION_INFO_COMPONENT_H_

/* Genode includes */
#include <util/list.h>

namespace Rtcr {
	struct Log_session_info;
	class Log_session_component;
}


/**
 * List element for monitoring session objects.
 * Each new connection from client to server is monitored here.
 */
struct Rtcr::Log_session_info : Genode::List<Log_session_info>::Element
{
	/**
	 * Reference to session object,
	 * encapsulates capability which is the main reason for storing it
	 */
	Log_session_component &session;
	/**
	 * Arguments provided for creating the session object
	 */
	const char            *args;

	Log_session_info(Log_session_component &comp, const char* args)
	:
		session(comp),
		args(args)
	{ }

	Log_session_info *find_by_ptr(Log_session_component *ptr)
	{
		if(ptr == &session)
			return this;
		Log_session_info *info = next();
		return info ? info->find_by_ptr(ptr) : nullptr;
	}
};

#endif /* _RTCR_LOG_SESSION_INFO_COMPONENT_H_ */
