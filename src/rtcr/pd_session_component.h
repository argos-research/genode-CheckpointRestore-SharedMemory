/*
 * \brief  Intercepting Pd session
 * \author Denis Huber
 * \date   2016-08-03
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

/**
 * This custom Pd session provides custom Region maps
 */
class Rtcr::Pd_session_component : public Genode::Rpc_object<Genode::Pd_session>
{
private:
	/**
	 * Enable log output for debugging
	 */
	static constexpr bool verbose_debug = true;

	/**
	 * TODO Needed?
	 */
	Genode::Env        &_env;
	/**
	 * TODO Needed?
	 */
	Genode::Allocator  &_md_alloc;
	/**
	 * TODO Needed?
	 */
	Genode::Entrypoint &_ep;
	/**
	 * Connection to parent's pd session, usually from core
	 */
	Genode::Pd_connection _parent_pd;

	/**
	 * Custom address space for monitoring the attachments of the Region map
	 */
	Rtcr::Region_map_component _address_space;
	/**
	 * Custom stack area for monitoring the attachments of the Region map
	 */
	Rtcr::Region_map_component _stack_area;
	/**
	 * custom linker area for monitoring the attachments of the Region map
	 */
	Rtcr::Region_map_component _linker_area;
	Genode::Entrypoint _pager_ep;

	void _handle()
	{
		Genode::log("handling!");
	}

public:
	/**
	 * Constructor
	 *
	 * \param env      Environment to create a session to parent's PD service
	 * \param md_alloc Allocator for the custom Region maps
	 * \param ep       Entrypoint for managing the custom Region maps
	 * \param label    Label for parent's PD session
	 */
	Pd_session_component(Genode::Env &env, Genode::Allocator &md_alloc, Genode::Entrypoint &ep, const char *label)
	:
		_env          (env),
		_md_alloc     (md_alloc),
		_ep           (ep),
		_parent_pd    (env, label),
		_address_space(_ep, _md_alloc, _parent_pd.address_space(), "address_space"),
		_stack_area   (_ep, _md_alloc, _parent_pd.stack_area(),    "stack_area"),
		_linker_area  (_ep, _md_alloc, _parent_pd.linker_area(),   "linker_area"),
		_pager_ep(env, 16*1024, "region_map pager_ep")
	{
		Genode::Signal_handler<Rtcr::Pd_session_component> sigh{_pager_ep, *this, &Rtcr::Pd_session_component::_handle};
		_address_space.fault_handler(sigh);
		if(verbose_debug) Genode::log("Pd_session_component created");
	}

	/**
	 * Destructor
	 */
	~Pd_session_component()
	{
		if(verbose_debug) Genode::log("Pd_session_component destroyed");
	}

	/**
	 * Return the capability to parent's PD session
	 */
	Genode::Pd_session_capability parent_cap()
	{
		return _parent_pd.cap();
	}

	/**
	 * Callback to signal that the child was created and that it attaches user-specific dataspaces
	 */
	void child_created()
	{
		_address_space.child_created();
		_stack_area.child_created();
		_linker_area.child_created();
	}

	/**************************
	 ** Pd_session interface **
	 **************************/

	void assign_parent(Genode::Capability<Genode::Parent> parent) override
	{
		if(verbose_debug) Genode::log("Pd::assign_parent()");
		_parent_pd.assign_parent(parent);
	}

	bool assign_pci(Genode::addr_t addr, Genode::uint16_t bdf) override
	{
		if(verbose_debug) Genode::log("Pd::assign_pci()");
		return _parent_pd.assign_pci(addr, bdf);
	}

	Signal_source_capability alloc_signal_source() override
	{
		if(verbose_debug) Genode::log("Pd::alloc_signal_source()");
		return _parent_pd.alloc_signal_source();
	}

	void free_signal_source(Signal_source_capability cap) override
	{
		if(verbose_debug) Genode::log("Pd::free_signal_source()");
		_parent_pd.free_signal_source(cap);
	}

	Genode::Capability<Genode::Signal_context> alloc_context(Signal_source_capability source,
			unsigned long imprint) override
	{
		if(verbose_debug) Genode::log("Pd::alloc_context()");
		return _parent_pd.alloc_context(source, imprint);
	}

	void free_context(Genode::Capability<Genode::Signal_context> cap) override
	{
		if(verbose_debug) Genode::log("Pd::free_context()");
		_parent_pd.free_context(cap);
	}

	void submit(Genode::Capability<Genode::Signal_context> context, unsigned cnt) override
	{
		if(verbose_debug) Genode::log("Pd::submit()");
		_parent_pd.submit(context, cnt);
	}

	Genode::Native_capability alloc_rpc_cap(Genode::Native_capability ep) override
	{
		if(verbose_debug)
		{
			Genode::log("Pd::alloc_rpc_cap()");
			Genode::log("  ep_cap:     ", ep.local_name());
		}

		Genode::Native_capability result {_parent_pd.alloc_rpc_cap(ep)};

		if(verbose_debug)
		{
			Genode::log("  result_cap: ", result.local_name());
		}
		return result;
	}

	void free_rpc_cap(Genode::Native_capability cap) override
	{
		if(verbose_debug)
		{
			Genode::log("Pd::free_rpc_cap()");
			Genode::log("  cap:        ", cap.local_name());
		}
		_parent_pd.free_rpc_cap(cap);
	}

	/**
	 * Return custom address space
	 */
	Genode::Capability<Genode::Region_map> address_space() override
	{
		if(verbose_debug) Genode::log("Pd::address_space()");
		return _address_space.Rpc_object<Genode::Region_map>::cap();
	}

	/**
	 * Return custom stack area
	 */
	Genode::Capability<Genode::Region_map> stack_area() override
	{
		if(verbose_debug) Genode::log("Pd::stack_area()");
		return _stack_area.Rpc_object<Genode::Region_map>::cap();
	}

	/**
	 * Return custom linker area
	 */
	Genode::Capability<Genode::Region_map> linker_area() override
	{
		if(verbose_debug) Genode::log("Pd::linker_area()");
		return _linker_area.Rpc_object<Genode::Region_map>::cap();
	}

	Genode::Capability<Native_pd> native_pd() override
	{
		if(verbose_debug) Genode::log("Pd::native_pd()");
		return _parent_pd.native_pd();
	}

};

#endif /* _RTCR_PD_SESSION_COMPONENT_H_ */
