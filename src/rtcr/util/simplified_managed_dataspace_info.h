/*
 * \brief  List element to store a managed dataspace and its designated dataspaces (simplified information)
 * \author Denis Huber
 * \date   2016-02-23
 */

#ifndef _RTCR_SIMPLIFIED_MANAGED_DATASPACE_INFO_H_
#define _RTCR_SIMPLIFIED_MANAGED_DATASPACE_INFO_H_

/* Genode includes */
#include <util/list.h>
#include <ram_session/ram_session.h>

/* Rtcr includes */


namespace Rtcr {
	struct Simplified_managed_dataspace_info;
}


/**
 * List element to store a managed dataspace and its designated dataspaces (simplified information)
 */
struct Rtcr::Simplified_managed_dataspace_info : Genode::List<Simplified_managed_dataspace_info>::Element
{
	/**
	 * List element to store simplified information of a designated dataspace
	 */
	struct Simplified_designated_ds_info : Genode::List<Simplified_designated_ds_info>::Element
	{
		Genode::Ram_dataspace_capability const dataspace_cap;
		Genode::addr_t const addr;
		Genode::size_t const size;
		bool modified;

		Simplified_designated_ds_info(Genode::Ram_dataspace_capability cap, Genode::addr_t addr, Genode::size_t size, bool modified)
		: dataspace_cap(cap), addr(addr), size(size), modified(modified) {}

		void print(Genode::Output &output) const
		{
			using Genode::Hex;

			Genode::print(output, "designated dataspace ", dataspace_cap, ", addr=", addr, ", size=", size);
		}
	};

	Genode::Ram_dataspace_capability            const region_map_dataspace_cap;
	Genode::List<Simplified_designated_ds_info> designated_dataspaces;

	Simplified_managed_dataspace_info(Genode::Ram_dataspace_capability cap, Genode::List<Simplified_designated_ds_info> &list)
	: region_map_dataspace_cap(cap), designated_dataspaces(list) { }

	Simplified_managed_dataspace_info *find_by_badge(Genode::uint16_t badge)
	{
		if(badge == region_map_dataspace_cap.local_name())
			return this;
		Simplified_managed_dataspace_info *info = next();
		return info ? info->find_by_badge(badge) : 0;
	}

	void print(Genode::Output &output) const
	{
		using Genode::Hex;

		Genode::print(output, "region map dataspace ", region_map_dataspace_cap);
	}
};

#endif /* _RTCR_SIMPLIFIED_MANAGED_DATASPACE_INFO_H_ */
