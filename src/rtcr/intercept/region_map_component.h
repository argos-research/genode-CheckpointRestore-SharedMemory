/*
 * \brief  Intercepting Region map
 * \author Denis Huber
 * \date   2016-08-09
 */

#ifndef _RTCR_REGION_MAP_COMPONENT_H_
#define _RTCR_REGION_MAP_COMPONENT_H_

/* Genode includes */
#include <base/log.h>
#include <base/allocator.h>
#include <base/entrypoint.h>
#include <region_map/client.h>
#include <dataspace/client.h>

/* Rtcr includes */
#include "ram_session_component.h"

namespace Rtcr {
	struct Attached_region_info;
	class Region_map_component;

	constexpr bool region_map_verbose_debug = false;
}

/**
 * Record of an attached dataspace
 */
struct Rtcr::Attached_region_info : public Genode::List<Attached_region_info>::Element
{
	// TODO comment these members
	Genode::Dataspace_capability ds_cap;
	Genode::size_t               size;
	Genode::off_t                offset;
	Genode::addr_t               addr;
	bool                         executable;

	/**
	 * Constructor
	 */
	Attached_region_info(Genode::Dataspace_capability ds_cap, Genode::size_t size,
			Genode::off_t offset, Genode::addr_t local_addr, bool executable);

	/**
	 * If this attached dataspace is managed, return its Managed_region_map_info, else return nullptr
	 */
	//Managed_region_map_info *managed_dataspace(Genode::List<Ram_dataspace_info> &rds_infos);
	Attached_region_info *find_by_addr(Genode::addr_t addr);
	Attached_region_info *find_by_cap(Genode::Dataspace_capability cap);
	//Attached_region_info *find_by_cr_info(Copied_region_info &cr_info);
};

/**
 * This custom Region map intercepts the attach and detach methods to monitor and
 * provide the content of this Region map
 */
class Rtcr::Region_map_component : public Genode::Rpc_object<Genode::Region_map>
{
private:
	/**
	 * Enable log output for debugging
	 */
	static constexpr bool verbose_debug = region_map_verbose_debug;

	/**
	 * Entrypoint which manages this object
	 */
	Genode::Entrypoint                 &_ep;
	/**
	 * Allocator for Region map's attachments
	 */
	Genode::Allocator                  &_md_alloc;
	/**
	 * Wrapped region map from parent, usually core
	 */
	Genode::Region_map_client           _parent_region_map;
	/**
	 * Parent's session state
	 */
	struct State_info
	{
		Genode::Signal_context_capability fault_handler {};
	} _parent_state;
	/**
	 * Name of the Region map for debugging
	 */
	Genode::String<32>                  _label;
	/**
	 * Lock to make _attached_regions thread-safe
	 */
	Genode::Lock                        _attached_regions_lock;
	/**
	 * List of client's dataspaces and their corresponding local addresses
	 */
	Genode::List<Attached_region_info>  _attached_regions;

public:
	/**
	 * Constructor
	 */
	Region_map_component(Genode::Entrypoint &ep, Genode::Allocator &md_alloc,
			Genode::Capability<Region_map> rm_cap, const char *label);
	/**
	 * Destrcutor
	 */
	~Region_map_component();

	Genode::Capability<Genode::Region_map> parent_cap()    { return _parent_region_map; }
	Genode::List<Attached_region_info> &attached_regions() { return _attached_regions;  }

	/******************************
	 ** Region map Rpc interface **
	 ******************************/

	/**
	 * Attaches a dataspace to parent's Region map and stores information about the attachment
	 */
	Local_addr attach(Genode::Dataspace_capability ds_cap, Genode::size_t size, Genode::off_t offset,
			bool use_local_addr, Region_map::Local_addr local_addr, bool executable) override;
	/**
	 * Detaches the dataspace from parent's region map and destroys the information about the attachment
	 */
	void detach(Region_map::Local_addr local_addr) override;
	void fault_handler(Genode::Signal_context_capability handler) override;
	State state() override;
	Genode::Dataspace_capability dataspace() override;
};

#endif /* _RTCR_REGION_MAP_COMPONENT_H_ */
