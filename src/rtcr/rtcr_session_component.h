/*
 * \brief  Rtcr session implementation
 * \author Denis Huber
 * \date   2016-08-26
 */

#include <base/allocator_guard.h>
#include <base/rpc_server.h>
#include <rtcr_session/rtcr_session.h>

#ifndef _RTCR__RTCR_SESSION_COMPONENT_H_
#define _RTCR__RTCR_SESSION_COMPONENT_H_

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
		if(verbose) Genode::log("checkpoint()");
		Genode::log(component.string());
	}

	void restore(Name const &component) override
	{
		if(verbose) Genode::log("restore()");
		Genode::log(component.string());
	}
};

#endif /* _RTCR__RTCR_SESSION_COMPONENT_H_ */
