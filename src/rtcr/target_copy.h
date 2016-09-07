/*
 * \brief  Child creation
 * \author Denis Huber
 * \date   2016-09-07
 */

#ifndef _RTCR_TARGET_COPY_H_
#define _RTCR_TARGET_COPY_H_

#include <util/list.h>

namespace Rtcr {
	class Target_copy;
}

class Rtcr::Target_copy : Genode::List<Target_copy>::Element
{
private:


public:
	Target_copy(Genode::List<Rtcr::Thread_info> &threads, Genode::List<Rtcr::Region_info> &attached_regions)
	{

	}

};

#endif /* _RTCR_TARGET_COPY_H_ */
