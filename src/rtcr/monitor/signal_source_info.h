/*
 * \brief  Monitoring signal source creation
 * \author Denis Huber
 * \date   2016-10-06
 */

#ifndef _RTCR_SIGNAL_SOURCE_INFO_COMPONENT_H_
#define _RTCR_SIGNAL_SOURCE_INFO_COMPONENT_H_

/* Genode includes */
#include <util/list.h>
#include <base/capability.h>

namespace Rtcr {
	struct Signal_source_info;
}


/**
 * List element to store Signal_source_capabilities created by the pd session
 */
struct Rtcr::Signal_source_info : Genode::List<Signal_source_info>::Element
{
	Genode::Capability<Genode::Signal_source> cap;

	Signal_source_info(Genode::Capability<Genode::Signal_source> cap)
	:
		cap(cap)
	{ }

	Signal_source_info *find_by_cap(Genode::Capability<Genode::Signal_source> cap)
	{
		if(cap == this->cap)
			return this;
		Signal_source_info *info = next();
		return info ? info->find_by_cap(cap) : 0;
	}
};

#endif /* _RTCR_SIGNAL_SOURCE_INFO_COMPONENT_H_ */
