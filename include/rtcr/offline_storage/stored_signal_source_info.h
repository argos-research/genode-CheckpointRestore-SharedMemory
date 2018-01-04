/*
 * \brief  Structure for storing signal context information
 * \author Denis Huber
 * \date   2016-10-26
 */

#ifndef _RTCR_STORED_SIGNAL_SOURCE_INFO_H_
#define _RTCR_STORED_SIGNAL_SOURCE_INFO_H_

/* Genode includes */
#include <util/list.h>

/* Rtcr includes */
#include "../online_storage/signal_source_info.h"
#include "../offline_storage/stored_info_structs.h"

namespace Rtcr {
	struct Stored_signal_source_info;
}


struct Rtcr::Stored_signal_source_info : Stored_normal_info, Genode::List<Stored_signal_source_info>::Element
{
	Stored_signal_source_info(Signal_source_info &info, Genode::addr_t targets_kcap)
	:
		Stored_normal_info(targets_kcap, info.cap.local_name(), info.bootstrapped)
	{ }

	Stored_signal_source_info(Genode::addr_t kcap,
                                        Genode::uint16_t local_name,
                                        bool bootstrapped)
	:
		Stored_normal_info(kcap, local_name, bootstrapped)
	{ }

	Stored_signal_source_info *find_by_badge(Genode::uint16_t badge)
	{
		if(badge == this->badge)
			return this;
		Stored_signal_source_info *info = next();
		return info ? info->find_by_badge(badge) : 0;
	}

	void print(Genode::Output &output) const
	{
		using Genode::Hex;

		Stored_normal_info::print(output);
	}

};

#endif /* _RTCR_STORED_SIGNAL_SOURCE_INFO_H_ */
