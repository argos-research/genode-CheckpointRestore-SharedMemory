/*
 * \brief  Intercepting Pd session
 * \author Denis Huber
 * \date   2016-08-03
 */

#include "pd_session_component.h"

using namespace Rtcr;


Pd_session_component::Pd_session_component(Genode::Env &env, Genode::Allocator &md_alloc, Genode::Entrypoint &ep, const char *label)
:
	_env           (env),
	_md_alloc      (md_alloc),
	_ep            (ep),
	_parent_pd     (env, label),
	_address_space (_ep, _md_alloc, _parent_pd.address_space(), "address_space"),
	_stack_area    (_ep, _md_alloc, _parent_pd.stack_area(),    "stack_area"),
	_linker_area   (_ep, _md_alloc, _parent_pd.linker_area(),   "linker_area"),
	_ss_infos_lock (),
	_ss_infos      (),
	_sc_infos_lock (),
	_sc_infos      (),
	_nc_infos_lock (),
	_nc_infos      ()
{
	if(verbose_debug) Genode::log("\033[33m", "Pd", "\033[0m (parent ", _parent_pd, ")");
}


Pd_session_component::~Pd_session_component()
{
	if(verbose_debug) Genode::log("\033[33m", "~Pd", "\033[0m ", _parent_pd);
}


void Pd_session_component::assign_parent(Genode::Capability<Genode::Parent> parent)
{
	if(verbose_debug) Genode::log("Pd::\033[33m", __func__, "\033[0m(", parent,")");

	_parent_pd.assign_parent(parent);
}


bool Pd_session_component::assign_pci(Genode::addr_t addr, Genode::uint16_t bdf)
{
	if(verbose_debug) Genode::log("Pd::\033[33m", __func__, "\033[0m(addr=", addr,", bdf=", bdf,")");

	auto result = _parent_pd.assign_pci(addr, bdf);

	if(verbose_debug) Genode::log("  result: ", result);

	return result;
}


Genode::Capability<Genode::Signal_source> Pd_session_component::alloc_signal_source()
{
	if(verbose_debug) Genode::log("Pd::\033[33m", __func__, "\033[0m()");

	auto result_cap = _parent_pd.alloc_signal_source();

	// Create and insert list element to monitor this signal source
	Signal_source_info *new_ss_info = new (_md_alloc) Signal_source_info(result_cap);
	Genode::Lock::Guard guard(_ss_infos_lock);
	_ss_infos.insert(new_ss_info);

	if(verbose_debug) Genode::log("  result: ", result_cap);

	return result_cap;
}


void Pd_session_component::free_signal_source(Genode::Capability<Genode::Signal_source> cap)
{
	if(verbose_debug) Genode::log("Pd::\033[33m", __func__, "\033[0m(", cap, ")");

	// Find list element
	Genode::Lock::Guard guard(_ss_infos_lock);
	Signal_source_info *ss_info = _ss_infos.first();
	if(ss_info) ss_info = ss_info->find_by_cap(cap);

	// List element found?
	if(ss_info)
	{
		// Remove and destroy list element
		_ss_infos.remove(ss_info);
		Genode::destroy(_md_alloc, ss_info);

		// Free signal source
		_parent_pd.free_signal_source(cap);
	}
	else
	{
		Genode::error("No list element found!");
	}
}


Genode::Signal_context_capability Pd_session_component::alloc_context(Signal_source_capability source,
		unsigned long imprint)
{
	if(verbose_debug) Genode::log("Pd::\033[33m", __func__, "\033[0m(source ", source, ", imprint=", Genode::Hex(imprint), ")");

	auto result_cap = _parent_pd.alloc_context(source, imprint);

	// Create and insert list element to monitor this signal context
	Signal_context_info *new_sc_info = new (_md_alloc) Signal_context_info(result_cap, source, imprint);
	Genode::Lock::Guard guard(_sc_infos_lock);
	_sc_infos.insert(new_sc_info);

	if(verbose_debug) Genode::log("  result: ", result_cap);

	return result_cap;
}


void Pd_session_component::free_context(Genode::Signal_context_capability cap)
{
	if(verbose_debug) Genode::log("Pd::\033[33m", __func__, "\033[0m(", cap, ")");

	// Find list element
	Genode::Lock::Guard guard(_sc_infos_lock);
	Signal_context_info *sc_info = _sc_infos.first();
	if(sc_info) sc_info = sc_info->find_by_sc_cap(cap);

	// List element found?
	if(sc_info)
	{
		// Remove and destroy list element
		_sc_infos.remove(sc_info);
		Genode::destroy(_md_alloc, sc_info);

		// Free signal context
		_parent_pd.free_context(cap);
	}
	else
	{
		Genode::error("No list element found!");
	}
}


void Pd_session_component::submit(Genode::Signal_context_capability context, unsigned cnt)
{
	if(verbose_debug) Genode::log("Pd::\033[33m", __func__, "\033[0m(context ", context, ", cnt=", cnt,")");

	_parent_pd.submit(context, cnt);
}


Genode::Native_capability Pd_session_component::alloc_rpc_cap(Genode::Native_capability ep)
{
	if(verbose_debug) Genode::log("Pd::\033[33m", "alloc_rpc_cap", "\033[0m(", ep, ")");

	auto result_cap = _parent_pd.alloc_rpc_cap(ep);

	// Create and insert list element to monitor this native_capability
	Native_capability_info *new_nc_info = new (_md_alloc) Native_capability_info(result_cap, ep);
	Genode::Lock::Guard guard(_nc_infos_lock);
	_nc_infos.insert(new_nc_info);

	if(verbose_debug) Genode::log("  result: ", result_cap);

	return result_cap;
}


void Pd_session_component::free_rpc_cap(Genode::Native_capability cap)
{
	if(verbose_debug) Genode::log("Pd::\033[33m", __func__, "\033[0m(", cap,")");

	// Find list element
	Genode::Lock::Guard guard(_nc_infos_lock);
	Native_capability_info *nc_info = _nc_infos.first();
	if(nc_info) nc_info = nc_info->find_by_native_cap(cap);

	// List element found?
	if(nc_info)
	{
		// Remove and destroy list element
		_nc_infos.remove(nc_info);
		Genode::destroy(_md_alloc, nc_info);

		// Free native capability
		_parent_pd.free_rpc_cap(cap);
	}
	else
	{
		Genode::error("No list element found!");
	}
}


Genode::Capability<Genode::Region_map> Pd_session_component::address_space()
{
	if(verbose_debug) Genode::log("Pd::\033[33m", __func__, "\033[0m()");

	auto result = _address_space.Rpc_object<Genode::Region_map>::cap();

	if(verbose_debug) Genode::log("  result: ", result);

	return result;
}


Genode::Capability<Genode::Region_map> Pd_session_component::stack_area()
{
	if(verbose_debug) Genode::log("Pd::\033[33m", __func__, "\033[0m()");

	auto result = _stack_area.Rpc_object<Genode::Region_map>::cap();

	if(verbose_debug) Genode::log("  result: ", result);

	return result;
}


Genode::Capability<Genode::Region_map> Pd_session_component::linker_area()
{
	if(verbose_debug) Genode::log("Pd::\033[33m", __func__, "\033[0m()");

	auto result = _linker_area.Rpc_object<Genode::Region_map>::cap();

	if(verbose_debug) Genode::log("  result: ", result);

	return result;
}


Genode::Capability<Genode::Pd_session::Native_pd> Pd_session_component::native_pd()
{
	if(verbose_debug) Genode::log("Pd::\033[33m", __func__, "\033[0m()");

	auto result = _parent_pd.native_pd();

	if(verbose_debug) Genode::log("  result: ", result);

	return result;
}
