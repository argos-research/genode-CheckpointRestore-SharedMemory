/*
 * \brief  Intercepting Rm session
 * \author Denis Huber
 * \date   2016-10-02
 */

#include "rm_session.h"

using namespace Rtcr;


Region_map_component &Rm_session_component::_create(Genode::size_t size)
{
	// Create real Region map from parent
	auto parent_cap = _parent_rm.create(size);

	// Create custom Region map
	Region_map_component *new_region_map =
			new (_md_alloc) Region_map_component(_md_alloc, parent_cap, size, "custom", _bootstrap_phase);

	// Manage custom Region map
	_ep.manage(*new_region_map);

	// Insert custom Region map into list
	Genode::Lock::Guard lock(_parent_state.region_maps_lock);
	_parent_state.region_maps.insert(new_region_map);

	return *new_region_map;
}


void Rm_session_component::_destroy(Region_map_component &region_map)
{
	// Reverse order as in _create
	auto parent_cap = region_map.parent_cap();

	// Remove custom RPC object form list
	Genode::Lock::Guard lock(_parent_state.region_maps_lock);
	_parent_state.region_maps.remove(&region_map);

	// Dissolve custom RPC object
	_ep.dissolve(region_map);

	// Destroy custom RPC object
	Genode::destroy(_md_alloc, &region_map);

	// Destroy real Region map from parent
	_parent_rm.destroy(parent_cap);
}


Rm_session_component::Rm_session_component(Genode::Env &env, Genode::Allocator &md_alloc, Genode::Entrypoint &ep,
		const char *creation_args, bool &bootstrap_phase)
:
	_md_alloc         (md_alloc),
	_ep               (ep),
	_bootstrap_phase  (bootstrap_phase),
	_parent_rm        (env),
	_parent_state     (creation_args, bootstrap_phase)
{
	if(verbose_debug) Genode::log("\033[33m", "Rm", "\033[0m(parent ", _parent_rm, ")");
}


Rm_session_component::~Rm_session_component()
{
	while(Region_map_component *obj = _parent_state.region_maps.first())
	{
		_destroy(*obj);
	}

	if(verbose_debug) Genode::log("\033[33m", "~Rm", "\033[0m ", _parent_rm);
}


Genode::Capability<Genode::Region_map> Rm_session_component::create(Genode::size_t size)
{
	if(verbose_debug) Genode::log("Rm::\033[33m", __func__, "\033[0m(size=", size, ")");

	// Create custom Region map
	Region_map_component new_region_map = _create(size);

	if(verbose_debug) Genode::log("  result: ", new_region_map.cap());
	return new_region_map.cap();
}


void Rm_session_component::destroy(Genode::Capability<Genode::Region_map> region_map_cap)
{
	if(verbose_debug) Genode::log("Rm::\033[33m", __func__, "\033[0m(", region_map_cap, ")");

	// Find RPC object for the given Capability
	Genode::Lock::Guard lock (_parent_state.region_maps_lock);
	Region_map_component *region_map = _parent_state.region_maps.first();
	if(region_map) region_map = region_map->find_by_badge(region_map_cap.local_name());

	// If found, delete everything concerning this RPC object
	if(region_map)
	{
		if(verbose_debug) Genode::log("  deleting ", region_map->cap());

		Genode::error("Issuing Rm_session::destroy, which is bugged and hangs up.");

		_destroy(*region_map);
	}
	else
	{
		Genode::error("No Region map with ", region_map_cap, " found!");
	}
}


Rm_session_component *Rm_root::_create_session(const char *args)
{
	if(verbose_debug) Genode::log("Rm_root::\033[33m", __func__, "\033[0m(", args,")");

	// Create custom Rm_session
	Rm_session_component *new_session =
			new (md_alloc()) Rm_session_component(_env, _md_alloc, _ep, args, _bootstrap_phase);

	Genode::Lock::Guard lock(_objs_lock);
	_session_rpc_objs.insert(new_session);

	return new_session;
}


void Rm_root::_destroy_session(Rm_session_component *session)
{
	_session_rpc_objs.remove(session);
	Genode::destroy(_md_alloc, session);
}


Rm_root::Rm_root(Genode::Env &env, Genode::Allocator &md_alloc, Genode::Entrypoint &session_ep,
		bool &bootstrap_phase)
:
	Root_component<Rm_session_component>(session_ep, md_alloc),
	_env              (env),
	_md_alloc         (md_alloc),
	_ep               (session_ep),
	_bootstrap_phase  (bootstrap_phase),
	_objs_lock        (),
	_session_rpc_objs ()
{
	if(verbose_debug) Genode::log("\033[33m", __func__, "\033[0m");
}

Rm_root::~Rm_root()
{
	while(Rm_session_component *obj = _session_rpc_objs.first())
	{
		_session_rpc_objs.remove(obj);
		Genode::destroy(_md_alloc, obj);
	}

	if(verbose_debug) Genode::log("\033[33m", __func__, "\033[0m");
}
