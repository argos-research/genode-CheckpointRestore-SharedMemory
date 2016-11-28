/*
 * \brief  Storing RAM dataspaces
 * \author Denis Huber
 * \date   2016-11-23
 */

#ifndef _RTCR_STORED_RAM_DATASPACE_INFO_H_
#define _RTCR_STORED_RAM_DATASPACE_INFO_H_

/* Genode includes */
#include <util/list.h>

/* Rtcr includes */
#include "stored_info_structs.h"
#include "../monitor/ram_dataspace_info.h"


namespace Rtcr {
	struct Stored_ram_dataspace_info;
}


struct Rtcr::Stored_ram_dataspace_info : Stored_normal_info, Genode::List<Stored_ram_dataspace_info>::Element
{
	Genode::Ram_dataspace_capability const memory_content;
	Genode::size_t                   const size;
	Genode::Cache_attribute          const cached;
	bool                             const managed;
	Genode::size_t                   const timestamp;

	Stored_ram_dataspace_info(Ram_dataspace_info &info, Genode::addr_t targets_kcap, Genode::Ram_dataspace_capability copy_ds_cap)
	:
		Stored_normal_info(targets_kcap,
				info.cap.local_name(),
				info.bootstrapped),
		memory_content(copy_ds_cap),
		size(info.size), cached(info.cached), managed(info.mrm_info),
		timestamp(info.timestamp())
	{
		Genode::log("  Ram_dataspace_info: ", info.timestamp(), " ", info.timestamp(), " ", info.timestamp());
		Genode::log("  Stored_ram_dataspace_info: ", timestamp);
	}

	Stored_ram_dataspace_info *find_by_badge(Genode::uint16_t badge)
	{
		if(badge == this->badge)
			return this;
		Stored_ram_dataspace_info *info = next();
		return info ? info->find_by_badge(badge) : 0;
	}

	Stored_ram_dataspace_info *find_by_timestamp(Genode::size_t timestamp)
	{
		if(timestamp == this->timestamp)
			return this;
		Stored_ram_dataspace_info *info = next();
		return info ? info->find_by_timestamp(timestamp) : 0;
	}

	void print(Genode::Output &output) const
	{
		using Genode::Hex;

		Stored_normal_info::print(output);
		Genode::print(output, ", size=", Hex(size), ", cached=", static_cast<unsigned>(cached),
				", managed=", managed, ", copy_ds ", memory_content, ", timestamp=", timestamp);
	}
};


#endif /* _RTCR_STORED_RAM_DATASPACE_INFO_H_ */
