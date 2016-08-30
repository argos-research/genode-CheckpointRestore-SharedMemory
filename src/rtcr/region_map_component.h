/*
 * \brief  Intercepting Region map
 * \author Denis Huber
 * \date   2016-08-09
 */

#ifndef _RTCR_REGION_MAP_COMPONENT_H_
#define _RTCR_REGION_MAP_COMPONENT_H_

#include <base/log.h>
#include <base/rpc_server.h>
#include <region_map/client.h>
#include <dataspace/client.h>

namespace Rtcr {
	struct Region_info;
	class Region_map_component;
}

/**
 * Record of an attached dataspace
 */
struct Rtcr::Region_info : public Genode::List<Region_info>::Element
{
	Genode::Dataspace_capability ds_cap;
	Genode::size_t               size;
	Genode::off_t                offset;
	Genode::addr_t               local_addr;
	bool                         executable;
	bool                         user_specific;

	/**
	 * Constructor
	 *
	 * Store necessary information of an attachment to this Region map to be able to recreate the Region map
	 *
	 * \param ds_cap        Capability to the attached dataspace
	 * \param size          Size of the dataspace
	 * \param offset        Offset in the dataspace
	 * \param local_addr    Address of the dataspace in parent's region map
	 * \param executable    Indicates whether the dataspace is executable
	 * \param user_specific Indicates whether the dataspace is created by the user who has written the child
	 *                      Dataspaces created before the user had control voer the program are not needed for checkpoint/restore
	 */
	Region_info(Genode::Dataspace_capability ds_cap, Genode::size_t size,
			Genode::off_t offset, Genode::addr_t local_addr, bool executable, bool user_specific)
	:
		ds_cap(ds_cap), size(size), offset(offset), local_addr(local_addr), executable(executable), user_specific(user_specific)
	{  }

	/**
	 * Find Region which contains the addr
	 *
	 * \param  addr Local address in parent's region map
	 * \return Region_info which contains the local address
	 */
	Region_info *find_by_addr(Genode::addr_t addr)
	{
		if((addr >= local_addr) && (addr <= local_addr + size))
			return this;
		Region_info *region = next();
		return region ? region->find_by_addr(addr) : 0;
	}
};

/**
 * This custom Region map intercepts the attach and detach methods to monitor and provide the content of this Region map
 */
class Rtcr::Region_map_component : public Genode::Rpc_object<Genode::Region_map>
{
private:
	/**
	 * Enable log output for debugging
	 */
	static constexpr bool verbose_debug = false;

	/**
	 * Entrypoint which manages this Region map
	 */
	Genode::Entrypoint        &_ep;
	/**
	 * Allocator for Region map's attachments
	 */
	Genode::Allocator         &_md_alloc;
	/**
	 * Wrapped region map from parent, usually core
	 */
	Genode::Region_map_client _parent_rm;
	/**
	 * Name of the Region map for debugging
	 */
	Genode::String<32>        _label;
	/**
	 * Indicates whether child has been already created or not
	 * The dataspaces from the child creation are not needed to be checkpointed/restored
	 */
	bool                      _child_created;
	/**
	 * List of target's local addresses and their corresponding dataspaces
	 */
	Genode::List<Region_info> _regions;
	/**
	 * Lock to make _regions thread-safe
	 */
	Genode::Lock              _regions_lock;

public:
	/**
	 * Constructor
	 *
	 * \param ep       Entrypoint for managing the custom Region map
	 * \param md_alloc Allocator for attachments
	 * \param rm_cap   Capability to parent's Region map
	 */
	Region_map_component(Genode::Entrypoint &ep, Genode::Allocator &md_alloc, Genode::Capability<Region_map> rm_cap, const char *label)
	:
		_ep(ep),
		_md_alloc(md_alloc),
		_parent_rm(rm_cap),
		_label(label),
		_child_created(false),
		_regions()
	{
		_ep.manage(*this);
		if(verbose_debug) Genode::log("Region_map_component created");
	}

	/**
	 * Destrcutor
	 */
	~Region_map_component()
	{
		_ep.dissolve(*this);

		// Destroy all Regions through detach method
		Region_info *curr;

		Genode::Lock::Guard lock_guard(_regions_lock);

		while((curr = _regions.first()))
			detach(curr->local_addr);
		if(verbose_debug) Genode::log("Region_map_component destroyed");
	}

	/**
	 * Let this Region map know, that the child is created and
	 * the dataspaces which will be attached shall be marked accordingly
	 *
	 * The dataspaces attached during child's creation to the address space are not subject for chcekpoint/restore
	 * They are recreated through the child's creation process
	 */
	void child_created() { _child_created = true; }

	/**
	 * Create a copy of a list containing regions attached to this Region map
	 *
	 * \param alloc Allocator where the new list shall be stored
	 */
	Genode::List<Region_info> copy_regions_list(Genode::Allocator &alloc)
	{
		Genode::List<Region_info> result;

		Region_info *curr = _regions.first();

		for(; curr; curr = curr->next())
		{
			Region_info *region = new (alloc) Region_info(
					curr->ds_cap, curr->size, curr->offset, curr->local_addr,
					curr->executable, curr->user_specific);
			result.insert(region);
		}

		return result;
	}

	/******************************
	 ** Region map Rpc interface **
	 ******************************/

	/**
	 * Attaches a dataspace to parent's Region map and stores information about the attachment
	 */
	Local_addr attach(Genode::Dataspace_capability ds_cap, Genode::size_t size, Genode::off_t offset,
			bool use_local_addr, Region_map::Local_addr local_addr, bool executable)
	{
		if(verbose_debug)
		{
			Genode::log("Rm::attach() - ", _label.string(), " child_created=", _child_created?"true":"false");
			Genode::log("  ","size=", size,
					", offset=", offset,
					", local_addr=", (void*)local_addr,
					", executable=", executable?"true":"false");
		}

		// Attach dataspace to real Region map
		Region_map::Local_addr addr = _parent_rm.attach(
				ds_cap, size, offset, use_local_addr, local_addr, executable);

		//Genode::log("  poke: ", (void*)addr," = ", (*(unsigned int*)addr));

		// Store information about the attachment
		Region_info *region = new (_md_alloc) Region_info(ds_cap, size, offset, addr, executable, _child_created);

		if(verbose_debug)
		{
			// Dataspace::size() returns a multiple of 4096 bytes (1 Pagesize)
			Genode::size_t ds_size = Genode::Dataspace_client(ds_cap).size();
			Genode::size_t num_pages = (Genode::size_t)(ds_size/4096);

			Genode::log("  Attached dataspace ", ds_cap.local_name(),
			" (local) into [", Genode::Hex((Genode::size_t)addr),
			", ", Genode::Hex((Genode::size_t)addr+ds_size), ") ",
			num_pages, num_pages==1?" page":" pages");
		}

		// Store Region_info in a list
		Genode::Lock::Guard lock_guard(_regions_lock);
		_regions.insert(region);

		return addr;
	}

	/**
	 * Detaches the dataspace from parent's region map and destroys the information about the attachment
	 */
	void detach(Region_map::Local_addr local_addr)
	{
		if(verbose_debug) Genode::log("Rm::detach() - ", _label.string());

		// Detach from real region map
		_parent_rm.detach(local_addr);

		// Find region
		Genode::Lock::Guard lock_guard(_regions_lock);
		Region_info *region = _regions.first()->find_by_addr((Genode::addr_t)local_addr);
		if(!region)
		{
			Genode::warning("Region not found in Rm::detach(). Local address ", (void*)local_addr,
					" not in regions list.");
			return;
		}

		// Remove and destroy region from list and allocator
		_regions.remove(region);
		destroy(_md_alloc, region);

		if(verbose_debug) Genode::log("  Detached dataspace for the local address ", (void*)local_addr);
	}

	void fault_handler(Genode::Signal_context_capability handler)
	{
		if(verbose_debug)
			Genode::log("Rm::fault_handler() - ", _label.string());

		_parent_rm.fault_handler(handler);
	}

	State state()
	{
		if(verbose_debug)
			Genode::log("Rm::state() - ", _label.string());

		return _parent_rm.state();
	}

	Genode::Dataspace_capability dataspace()
	{
		if(verbose_debug)
			Genode::log("Rm::dataspace() - ", _label.string());

		return _parent_rm.dataspace();
	}
};

#endif /* _RTCR_REGION_MAP_COMPONENT_H_ */
