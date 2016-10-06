/*
 * \brief  Intercepting Rm session
 * \author Denis Huber
 * \date   2016-10-02
 */

#include "rm_session.h"

using namespace Rtcr;


Region_map_info::Region_map_info(Region_map_component &region_map)
:
	region_map(region_map)
{ }


Region_map_info *Region_map_info::find_by_cap(Genode::Capability<Genode::Region_map> cap)
{
	if(cap == region_map.cap())
		return this;
	Region_map_info *rm_info = next();
	return rm_info ? rm_info->find_by_cap(cap) : 0;
}


Rm_session_info::Rm_session_info(Rm_session_component &rms, const char* args)
:
	rms  (rms),
	args (args)
{ }


Rm_session_info *Rm_session_info::find_by_ptr(Rm_session_component *ptr)
{
	if(ptr == &rms)
		return this;
	Rm_session_info *rms_info = next();
	return rms_info ? rms_info->find_by_ptr(ptr) : 0;
}


Rm_session_component::Rm_session_component(Genode::Env &env, Genode::Allocator &md_alloc, Genode::Entrypoint &ep)
:
	_md_alloc         (md_alloc),
	_ep               (ep),
	_parent_rm        (env),
	_infos_lock       (),
	_region_map_infos ()
{
	if(verbose_debug) Genode::log("\033[33m", __func__, "\033[0m");
}


Rm_session_component::~Rm_session_component()
{
	// Destroy all list elements through destroy method
	Region_map_info *rms_info = nullptr;
	while((rms_info = _region_map_infos.first()))
	{
		_region_map_infos.remove(rms_info);
		destroy(rms_info->region_map.cap());
	}

	if(verbose_debug) Genode::log("\033[33m", __func__, "\033[0m");
}


Genode::Capability<Genode::Region_map> Rm_session_component::create(Genode::size_t size)
{
	if(verbose_debug) Genode::log("Rm::\033[33m", __func__, "\033[0m(size=", size, ")");

	// Create real Region_map from parent
	auto parent_cap = _parent_rm.create(size);

	// Create virtual Region_map
	Region_map_component *new_region_map =
			new (_md_alloc) Region_map_component(_ep, _md_alloc, parent_cap, "custom");

	// Create list element to where the virtual object is stored
	Region_map_info *rm_info = new (_md_alloc) Region_map_info(*new_region_map);

	// Insert list element into list
	Genode::Lock::Guard lock(_infos_lock);
	_region_map_infos.insert(rm_info);

	if(verbose_debug) Genode::log("  result: ", new_region_map->cap());
	return new_region_map->cap();
}


void Rm_session_component::destroy(Genode::Capability<Genode::Region_map> region_map_cap)
{
	if(verbose_debug) Genode::log("Rm::\033[33m", __func__, "\033[0m(", region_map_cap, ")");

	// Find list element for the given Capability
	Genode::Lock::Guard lock (_infos_lock);
	Region_map_info *rms_info = _region_map_infos.first();
	if(rms_info) rms_info = rms_info->find_by_cap(region_map_cap);

	// If found, delete everything concerning this list element
	if(rms_info)
	{
		if(verbose_debug) Genode::log("  deleting ", rms_info->region_map.cap());

		Genode::error("Issuing Rm_session::destroy, which is bugged and hangs up.");
		// Destroy real Region_map from parent
		_parent_rm.destroy(rms_info->region_map.parent_cap());

		// Destroy virtual Region_map
		Genode::destroy(_md_alloc, &rms_info->region_map);

		// Remove and destroy list element
		_region_map_infos.remove(rms_info);
		Genode::destroy(_md_alloc, rms_info);
	}
	else
	{
		Genode::error("No Region map with ", region_map_cap, " found!");
	}
}


Rm_session_component *Rm_root::_create_session(const char *args)
{
	if(verbose_debug) Genode::log("Rm_root::\033[33m", __func__, "\033[0m(", args,")");
	// Create virtual Rm_session
	Rm_session_component *new_rms =
			new (md_alloc()) Rm_session_component(_env, _md_alloc, _ep);

	// Create and insert list element
	Rm_session_info *new_rms_info =
			new (md_alloc()) Rm_session_info(*new_rms, args);
	Genode::Lock::Guard lock(_infos_lock);
	_rms_infos.insert(new_rms_info);

	return new_rms;
}


void Rm_root::_destroy_session(Rm_session_component *session)
{
	// Find and destroy list element
	Rm_session_info *rms_info = _rms_infos.first();
	if(rms_info) rms_info = rms_info->find_by_ptr(session);
	if(rms_info)
	{
		_rms_infos.remove(rms_info);
		destroy(_md_alloc, rms_info);
	}

	// Destroy virtual Rm_session
	destroy(_md_alloc, session);
}


Rm_root::Rm_root(Genode::Env &env, Genode::Allocator &md_alloc, Genode::Entrypoint &session_ep)
:
	Root_component<Rm_session_component>(session_ep, md_alloc),
	_env        (env),
	_md_alloc   (md_alloc),
	_ep         (session_ep),
	_infos_lock (),
	_rms_infos  ()
{
	if(verbose_debug) Genode::log("\033[33m", __func__, "\033[0m");
}

Rm_root::~Rm_root()
{
	Rm_session_info *info = nullptr;

	while((info = _rms_infos.first()))
	{
		_rms_infos.remove(info);
		Genode::destroy(_md_alloc, info);
	}

	if(verbose_debug) Genode::log("\033[33m", __func__, "\033[0m");
}
