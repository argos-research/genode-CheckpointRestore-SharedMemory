/*
 * \brief  Storing dataspaces
 * \author Denis Huber
 * \date   2016-10-25
 */

#ifndef _RTCR_STORED_DATASPACE_INFO_H_
#define _RTCR_STORED_DATASPACE_INFO_H_

/* Genode includes */
#include <util/list.h>

/* Rtcr includes */
#include "../monitor/ram_dataspace_info.h"
#include "../monitor/attached_region_info.h"


namespace Rtcr {
	struct Stored_dataspace_info;
}


struct Rtcr::Stored_dataspace_info : Genode::List<Stored_dataspace_info>::Element
{
	/**
	 * Child's kcap (kernel capability selector)
	 */
	Genode::addr_t               kcap;
	/**
	 * Genode's system-global capability identifier
	 */
	Genode::uint16_t             badge;
	/**
	 * Content of dataspace
	 */
	Genode::Dataspace_capability ds_cap;
	Genode::size_t               size;
	Genode::Cache_attribute      cached;
	bool                         managed;

	Stored_dataspace_info()
	:
		kcap(0), badge(0), ds_cap(Genode::Dataspace_capability()),
		size(0), cached(Genode::Cache_attribute::UNCACHED), managed(false)
	{ }

	Stored_dataspace_info(Ram_dataspace_info &info)
	:
		kcap(0), badge(info.ds_cap.local_name()), ds_cap(Genode::Dataspace_capability()),
		size(info.size), cached(info.cached), managed(info.mrm_info)
	{ }

	Stored_dataspace_info(Attached_region_info &info)
	:
		kcap(0), badge(info.ds_cap.local_name()), ds_cap(Genode::Dataspace_capability()),
		size(info.size), cached(Genode::Cache_attribute::UNCACHED), managed(false)
	{ }

	Stored_dataspace_info *find_by_badge(Genode::uint16_t badge)
	{
		if(badge == this->badge)
			return this;
		Stored_dataspace_info *info = next();
		return info ? info->find_by_badge(badge) : 0;
	}

	void print(Genode::Output &output) const
	{
		using Genode::Hex;

		Genode::print(output, "<", Hex(kcap), ",", badge, "> ",
				ds_cap,
				" size=", size,
				" cached=", static_cast<unsigned>(cached),
				" man=", managed?"y":"n");
	}
};


#endif /* _RTCR_STORED_DATASPACE_INFO_H_ */
