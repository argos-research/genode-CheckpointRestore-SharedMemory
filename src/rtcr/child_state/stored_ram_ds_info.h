/*
 * \brief  Storing RAM dataspaces
 * \author Denis Huber
 * \date   2016-10-25
 */

#ifndef _RTCR_STORED_RAM_DS_INFO_COMPONENT_H_
#define _RTCR_STORED_RAM_DS_INFO_COMPONENT_H_

/* Genode includes */
#include <util/list.h>

namespace Rtcr {
	struct Stored_ram_ds_info;
}


struct Rtcr::Stored_ram_ds_info : Genode::List<Stored_ram_ds_info>::Element
{
	/**
	 * Child's kcap (kernel capability selector)
	 */
	Genode::addr_t          kcap;
	/**
	 * Genode's system-global capability identifier
	 */
	Genode::uint16_t        badge;
	// TODO include stored state of ram dataspace!
	Genode::size_t          size;
	Genode::Cache_attribute cached;
	bool                    managed;

	Stored_ram_ds_info(Genode::Ram_dataspace_capability ram_ds_cap, Genode::size_t size,
			Genode::Cache_attribute cached, bool managed)
	:
		kcap    (0),
		badge   (ram_ds_cap.local_name()),
		size    (size),
		cached  (cached),
		managed (managed)
	{ }

	Stored_ram_ds_info *find_by_badge(Genode::uint16_t badge)
	{
		if(badge == this->badge)
			return this;
		Stored_ram_ds_info *info = next();
		return info ? info->find_by_badge(badge) : 0;
	}
};


#endif /* _RTCR_STORED_RAM_DS_INFO_COMPONENT_H_ */
