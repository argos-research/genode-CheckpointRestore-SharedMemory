/*
 * \brief  Monitoring RM::attach and RM::detach
 * \author Denis Huber
 * \date   2016-10-06
 */

#ifndef _RTCR_ATTACHED_REGION_INFO_H_
#define _RTCR_ATTACHED_REGION_INFO_H_

/* Genode includes */
#include <util/list.h>
#include <dataspace/capability.h>

/* Rtcr includes */
//#include "info_structs.h"
#include "ram_dataspace_info.h"

namespace Rtcr {
	struct Attached_region_info;
}

/**
 * Record of an attached dataspace
 */
struct Rtcr::Attached_region_info : Normal_obj_info, Genode::List<Attached_region_info>::Element
{
	/**
	 * Dataspace capability which is attached
	 */
	const Genode::Dataspace_capability attached_ds_cap;
	/**
	 * Size of occupied region
	 */
	const Genode::size_t size;
	/**
	 * Offset in occupied region
	 */
	const Genode::off_t  offset;
	/**
	 * Address of occupied region
	 */
	const Genode::addr_t rel_addr;
	/**
	 * Indicates whether occupied region is executable
	 */
	const bool           executable;


	Attached_region_info(Genode::Dataspace_capability attached_ds_cap, Genode::size_t size, Genode::off_t offset,
			Genode::addr_t local_addr, bool executable, bool bootstrapped = false)
	:
		Normal_obj_info (bootstrapped),
		attached_ds_cap (attached_ds_cap),
		size       (size),
		offset     (offset),
		rel_addr   (local_addr),
		executable (executable)
	{ }

	/**
	 * If this attached dataspace is managed, return its Managed_region_map_info, else return nullptr
	 */
	Managed_region_map_info *managed_dataspace(Genode::List<Ram_dataspace_info> &rds_infos)
	{
		Ram_dataspace_info *rds_info = rds_infos.first();
		if(rds_info) rds_info = rds_info->find_by_cap(attached_ds_cap);
		return rds_info ? rds_info->mrm_info : 0;
	}
	Attached_region_info *find_by_addr(Genode::addr_t addr)
	{
		if((addr >= rel_addr) && (addr <= rel_addr + size))
			return this;
		Attached_region_info *info = next();
		return info ? info->find_by_addr(addr) : 0;
	}
	Attached_region_info *find_by_cap(Genode::Dataspace_capability cap)
	{
		if(cap == attached_ds_cap)
			return this;
		Attached_region_info *info = next();
		return info ? info->find_by_cap(cap) : 0;
	}
	Attached_region_info *find_by_badge(Genode::uint16_t badge)
	{
		if(badge == attached_ds_cap.local_name())
			return this;
		Attached_region_info *info = next();
		return info ? info->find_by_badge(badge) : 0;
	}
	Attached_region_info *find_by_cap_and_addr(Genode::Dataspace_capability cap, Genode::addr_t addr)
	{
		if(cap == attached_ds_cap && addr == rel_addr)
			return this;
		Attached_region_info *info = next();
		return info ? info->find_by_cap_and_addr(cap, addr) : 0;
	}
	Attached_region_info *find_by_addr_and_exec(Genode::addr_t addr, bool exec)
	{
		if((exec == executable) && (addr >= rel_addr) && (addr <= rel_addr + size))
			return this;
		Attached_region_info *info = next();
		return info ? info->find_by_addr_and_exec(addr, exec) : 0;
	}


	void print(Genode::Output &output) const
	{
		using Genode::Hex;
		using Genode::print;

		print(output, attached_ds_cap);
		print(output, " [", Hex(rel_addr, Hex::PREFIX, Hex::PAD));
		print(output, ", ", Hex(rel_addr + size - offset, Hex::PREFIX, Hex::PAD));
		print(output, ") exec=", executable, ", ");
		Normal_obj_info::print(output);
	}
};


#endif /* _RTCR_ATTACHED_REGION_INFO_H_ */
