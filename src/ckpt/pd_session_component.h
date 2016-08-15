/*
 * \brief Intercepting Pd session
 * \author Denis Huber
 * \date 2016-08-03
 */

#ifndef _RTCR_PD_SESSION_COMPONENT_H_
#define _RTCR_PD_SESSION_COMPONENT_H_

/* Genode includes */
#include <base/log.h>
#include <base/rpc_server.h>
#include <pd_session/connection.h>

/* Rtcr includes */
#include "region_map_component.h"

namespace Rtcr {
	class Pd_session_component;
}

class Rtcr::Pd_session_component : public Genode::Rpc_object<Genode::Pd_session>
{
private:
	static constexpr bool verbose = true;

	Genode::Env        &_env;
	Genode::Entrypoint &_ep;
	Genode::Allocator  &_md_alloc;
	/**
	 * Connection to parent's pd connection, usually from core
	 */
	Genode::Pd_connection _parent_pd;

	Region_map_component _address_space;
	Region_map_component _stack_area;
	Region_map_component _linker_area;

public:
	/**
	 * Constructor
	 */
	Pd_session_component(Genode::Env &env, Genode::Entrypoint &ep, Genode::Allocator &md_alloc, const char *label)
	:
		_env(env),
		_ep(ep),
		_md_alloc(md_alloc),
		_parent_pd(env, label),
		_address_space(),
		_stack_area(),
		_linker_area()
	{
		_ep.manage(*this);
		if(verbose)
		{
			Genode::log("Pd_session_component created");
		}
	}

	~Pd_session_component()
	{
		_ep.dissolve(*this);
		if(verbose) Genode::log("Pd_session_component destroyed");
	}

	Genode::Pd_session_capability parent_pd_cap()
	{
		return _parent_pd.cap();
	}

	/**************************
	 ** Pd_session interface **
	 **************************/

	void assign_parent(Genode::Capability<Genode::Parent> parent) override
	{
		if(verbose) Genode::log("Pd::assign_parent()");
		_parent_pd.assign_parent(parent);
	}

	bool assign_pci(Genode::addr_t addr, Genode::uint16_t bdf) override
	{
		if(verbose) Genode::log("Pd::assign_pci()");
		return _parent_pd.assign_pci(addr, bdf);
	}

	Signal_source_capability alloc_signal_source() override
	{
		if(verbose) Genode::log("Pd::alloc_signal_source()");
		return _parent_pd.alloc_signal_source();
	}

	void free_signal_source(Signal_source_capability cap) override
	{
		if(verbose) Genode::log("Pd::free_signal_source()");
		_parent_pd.free_signal_source(cap);
	}

	Genode::Capability<Genode::Signal_context> alloc_context(Signal_source_capability source,
			unsigned long imprint) override
	{
		if(verbose) Genode::log("Pd::alloc_context()");
		return _parent_pd.alloc_context(source, imprint);
	}

	void free_context(Genode::Capability<Genode::Signal_context> cap) override
	{
		if(verbose) Genode::log("Pd::free_context()");
		_parent_pd.free_context(cap);
	}

	void submit(Genode::Capability<Genode::Signal_context> context, unsigned cnt) override
	{
		if(verbose) Genode::log("Pd::submit()");
		_parent_pd.submit(context, cnt);
	}

	Genode::Native_capability alloc_rpc_cap(Genode::Native_capability ep) override
	{
		if(verbose) Genode::log("Pd::alloc_rpc_cap()");
		return _parent_pd.alloc_rpc_cap(ep);
	}

	void free_rpc_cap(Genode::Native_capability cap) override
	{
		if(verbose) Genode::log("Pd::free_rpc_cap()");
		_parent_pd.free_rpc_cap(cap);
	}

	Genode::Capability<Genode::Region_map> address_space() override
	{
		if(verbose) Genode::log("Pd::address_space()");
		return _parent_pd.address_space();
	}

	Genode::Capability<Genode::Region_map> stack_area() override
	{
		if(verbose) Genode::log("Pd::stack_area()");
		return _parent_pd.stack_area();
	}

	Genode::Capability<Genode::Region_map> linker_area() override
	{
		if(verbose) Genode::log("Pd::linker_area()");
		return _parent_pd.linker_area();
	}

	Genode::Capability<Native_pd> native_pd() override
	{
		if(verbose) Genode::log("Pd::native_pd()");
		return _parent_pd.native_pd();
	}

};

#endif /* _RTCR_PD_SESSION_COMPONENT_H_ */
