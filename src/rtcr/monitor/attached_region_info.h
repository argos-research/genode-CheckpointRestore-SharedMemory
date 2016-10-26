/*
 * \brief  Monitoring dataspace attaching/detaching
 * \author Denis Huber
 * \date   2016-10-06
 */

#ifndef _RTCR_ATTACHED_REGION_INFO_COMPONENT_H_
#define _RTCR_ATTACHED_REGION_INFO_COMPONENT_H_

/* Genode includes */
#include <util/list.h>
#include <dataspace/capability.h>

/* Rtcr includes */
#include "ram_dataspace_info.h"

namespace Rtcr {
	struct Attached_region_info;
}

/**
 * Record of an attached dataspace
 */
struct Rtcr::Attached_region_info : public Genode::List<Attached_region_info>::Element
{
	/**
	 * Dataspace capability which is attached
	 */
	Genode::Dataspace_capability ds_cap;
	/**
	 * Size of occupied region
	 */
	Genode::size_t               size;
	/**
	 * Offset in occupied region
	 */
	Genode::off_t                offset;
	/**
	 * Address of occupied region
	 */
	Genode::addr_t               rel_addr;
	/**
	 * Indicates whether occupied region is executable
	 */
	bool                         executable;


	Attached_region_info(Genode::Dataspace_capability ds_cap, Genode::size_t size,
			Genode::off_t offset, Genode::addr_t local_addr, bool executable)
	:
		ds_cap(ds_cap), size(size), offset(offset), rel_addr(local_addr), executable(executable)
	{ }

	/**
	 * If this attached dataspace is managed, return its Managed_region_map_info, else return nullptr
	 */
	Managed_region_map_info *managed_dataspace(Genode::List<Ram_dataspace_info> &rds_infos)
	{
		Ram_dataspace_info *rds_info = rds_infos.first();
		if(rds_info) rds_info = rds_info->find_by_cap(ds_cap);
		return rds_info ? rds_info->mrm_info : nullptr;
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
		if(cap == ds_cap)
			return this;
		Attached_region_info *info = next();
		return info ? info->find_by_cap(cap) : 0;
	}
	Attached_region_info *find_by_cap_and_addr(Genode::Dataspace_capability cap, Genode::addr_t addr)
	{
		if(cap == ds_cap && addr == rel_addr)
			return this;
		Attached_region_info *info = next();
		return info ? info->find_by_cap_and_addr(cap, addr) : 0;
	}

	void print(Genode::Output &output) const
	{
		using Genode::Hex;

		Genode::print(output, ds_cap);
		Genode::print(output, " [");
		Genode::print(output, Hex(rel_addr, Hex::PREFIX, Hex::PAD));
		Genode::print(output, ", ");
		Genode::print(output, Hex(rel_addr + size, Hex::PREFIX, Hex::PAD));
		Genode::print(output, ")");
		Genode::print(output, executable ? " exec" : "");
	}
};


#endif /* _RTCR_ATTACHED_REGION_INFO_COMPONENT_H_ */
