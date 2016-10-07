/*
 * \brief  Monitoring native capability creation
 * \author Denis Huber
 * \date   2016-10-06
 */

#ifndef _RTCR_NATIVE_CAPABILITY_INFO_COMPONENT_H_
#define _RTCR_NATIVE_CAPABILITY_INFO_COMPONENT_H_

/* Genode includes */
#include <util/list.h>
#include <base/native_capability.h>

namespace Rtcr {
	struct Native_capability_info;
}

/**
 * List element to store a capability which is created by the pd session
 * They are usually created by client's entrypoint and therefore require
 * a cpu_thread_capability
 */
struct Rtcr::Native_capability_info : Genode::List<Native_capability_info>::Element
{
	Genode::Native_capability native_cap;
	Genode::Native_capability ep_cap;

	Native_capability_info(Genode::Native_capability native_cap, Genode::Native_capability ep_cap)
	:
		native_cap(native_cap),
		ep_cap(ep_cap)
	{ }

	Native_capability_info *find_by_native_cap(Genode::Native_capability cap)
	{
		if(cap == this->native_cap)
			return this;
		Native_capability_info *info = next();
		return info ? info->find_by_native_cap(cap) : 0;
	}
};


#endif /* _RTCR_NATIVE_CAPABILITY_INFO_COMPONENT_H_ */
