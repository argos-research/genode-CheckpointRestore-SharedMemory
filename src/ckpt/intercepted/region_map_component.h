/*
 * \brief Intercepting Region map
 * \author Denis Huber
 * \date 2016-08-09
 */

#ifndef _RTCR_REGION_MAP_COMPONENT_H_
#define _RTCR_REGION_MAP_COMPONENT_H_

#include <base/log.h>
#include <base/entrypoint.h>
#include <base/rpc_server.h>
#include <base/allocator.h>
#include <region_map/client.h>
#include <region_map/region_map.h>

namespace Rtcr {
	class Region_map_component;
	using namespace Genode;
}

class Rtcr::Region_map_component : public Rpc_object<Region_map>
{
private:
	static constexpr bool verbose = true;

	Env               &_env;
	Allocator         &_md_alloc;
	Entrypoint        &_ep;
	/**
	 * Wrapped region map at core
	 */
	Region_map_client _parent_region_map;

public:
	/**
	 * Record of an attached dataspace
	 */
	class Region : public List<Region>::Element
	{
	private:
		void                *_start;
		void                *_end;
		off_t                _offset;
		Dataspace_capability _ds_cap;

	public:
		Region(void *start, void *end, off_t offset, Dataspace_capability ds_cap)
		:
			_start(start), _end(end), _offset(offset), _ds_cap(ds_cap)
		{}

		/**
		 * Find Region which contains the addr
		 */
		Region *find_by_addr(void *addr)
		{
			if((addr >= _start) && (addr <= _end))
				return this;
			Region *region = next();
			return region ? region->find_by_addr(addr) : 0;
		}

		void *local_addr() { return _start; }
	};

private:

	/**
	 * Mapping of target's local addresses and dataspaces
	 */
	List<Region> _regions;
	Lock         _region_map_lock;

public:
	/**
	 * Constructor
	 */
	Region_map_component(Env &env, Allocator &md_alloc, Entrypoint &ep,
			Capability<Region_map> parent_region_map)
	:
		_env(env), _md_alloc(md_alloc), _ep(ep),
		_parent_region_map(parent_region_map), _regions()
	{
		_ep.manage(*this);
	}

	~Region_map_component()
	{
		_ep.dissolve(*this);

		// detach all dataspaces
		Region *curr;
		while((curr = _regions.first()))
			detach(curr->local_addr());
	}

	/******************************
	 ** Region map Rpc interface **
	 ******************************/

	Local_addr           attach        (Dataspace_capability, size_t,
	                                    off_t, bool, Local_addr, bool) override;
	void                 detach        (Local_addr) override;
	void                 fault_handler (Signal_context_capability) override;
	State                state         () override;
	Dataspace_capability dataspace     () override;
};

#endif /* _RTCR_REGION_MAP_COMPONENT_H_ */
