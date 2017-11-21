/*
 * \brief  Rtcr session implementation
 * \author Denis Huber
 * \date   2016-08-26
 */

#ifndef _RTCR__RTCR_SESSION_COMPONENT_H_
#define _RTCR__RTCR_SESSION_COMPONENT_H_

#include <base/allocator_guard.h>
#include <base/rpc_server.h>
#include <rtcr_session/rtcr_session.h>

namespace Rtcr {
	class Session_component;
}

class Rtcr::Session_component : public Genode::Rpc_object<Rtcr::Session>
{
private:
	static constexpr bool verbose = true;

public:
	Session_component()  { }

	~Session_component() { }

	/*****************************
	 ** Rtcr::Session interface **
	 *****************************/

	void checkpoint(Name const &component) override
	{
		if(verbose) Genode::log("checkpoint(component=", component.string(),")");
	}

	void restore(Name const &component) override
	{
		if(verbose) Genode::log("restore(component=", component.string(),")");
	}
};

#endif /* _RTCR__RTCR_SESSION_COMPONENT_H_ */
