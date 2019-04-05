/*
 * \brief  Intercepting Region map
 * \author Denis Huber
 * \date   2016-08-09
 */

#ifndef _RTCR_REGION_MAP_COMPONENT_H_
#define _RTCR_REGION_MAP_COMPONENT_H_

/* Genode includes */
#include <region_map/client.h>
#include <dataspace/client.h>
#include <base/allocator.h>
#include <base/session_object.h>

/* Rtcr includes */
#include "ram_session.h"
#include "../online_storage/attached_region_info.h"
#include "../online_storage/region_map_info.h"

namespace Rtcr {
	class Region_map_component;

	constexpr bool region_map_verbose_debug = false;
}

/**
 * Custom Region map intercepting RPC methods
 */
class Rtcr::Region_map_component : public Genode::Session_object<Genode::Region_map>,
                                   private Genode::List<Region_map_component>::Element
{
private:
	friend class Genode::List<Rtcr::Region_map_component>;

	Region_map_component(Region_map_component const&) = default;
        Region_map_component& operator=(Region_map_component const&) = default;
	/**
	 * Enable log output for debugging
	 */
	static constexpr bool verbose_debug = region_map_verbose_debug;

	/**
	 * Allocator for Region map's attachments
	 */
	Genode::Allocator         &_md_alloc;
	/**
	 * Reference to Target_child's bootstrap phase
	 */
	bool                      &_bootstrap_phase;
	/**
	 * Name of the Region map for debugging
	 */
	const char*                _label;
	/**
	 * Wrapped region map from parent, usually core
	 */
	Genode::Region_map_client  _parent_region_map;
	/**
	 * State of parent's RPC object
	 */
	Region_map_info            _parent_state;

public:
	Region_map_component(Genode::Allocator &md_alloc, Genode::Capability<Genode::Region_map> region_map_cap,
			Genode::size_t size, const char *label, bool &bootstrap_phase, Resources resources, Diag diag, Genode::Entrypoint &ep);
	~Region_map_component();

	Genode::Capability<Genode::Region_map> parent_cap() { return _parent_region_map; }

	Region_map_info &parent_state() { return _parent_state; }
	Region_map_info const &parent_state() const { return _parent_state; }

	Region_map_component *find_by_badge(Genode::uint16_t badge);

	using Genode::List<Rtcr::Region_map_component>::Element::next;

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
