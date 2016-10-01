/*
 * \brief  General info for capabilities
 * \author Denis Huber
 * \date   2016-10-01
 */

#ifndef _RTCR_CAPABILITY_INFO_H_
#define _RTCR_CAPABILITY_INFO_H_

namespace Rtcr {
	class Capability_info;
}

class Rtcr::Capability_info : Genode::List<Capability_info>::Element
{
	Capability_info() { }
};

#endif /* _RTCR_CAPABILITY_INFO_H_ */
