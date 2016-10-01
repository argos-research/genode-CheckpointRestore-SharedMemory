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

	constexpr bool pd_verbose_debug = true;
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
	static constexpr bool verbose_debug = pd_verbose_debug;

	/**
	 * TODO Needed?
	 */
	Genode::Env                &_env;
	/**
	 * TODO Needed?
	 */
	Genode::Allocator          &_md_alloc;
	/**
	 * Entrypoint to manage itself
	 */
	Genode::Entrypoint         &_ep;
	/**
	 * Connection to parent's pd session, usually from core
	 */
	Genode::Pd_connection       _parent_pd;
	/**
	 * Custom address space for monitoring the attachments of the Region map
	 */
	Rtcr::Region_map_component  _address_space;
	/**
	 * Custom stack area for monitoring the attachments of the Region map
	 */
	Rtcr::Region_map_component  _stack_area;
	/**
	 * custom linker area for monitoring the attachments of the Region map
	 */
	Rtcr::Region_map_component  _linker_area;


public:
	/**
	 * Constructor
	 *
	 * \param env      Environment to create a session to parent's PD service
	 * \param md_alloc Allocator for the custom Region maps
	 * \param ep       Entrypoint for managing the custom Region maps and itself
	 * \param label    Label for parent's PD session
	 */
	Pd_session_component(Genode::Env &env, Genode::Allocator &md_alloc, Genode::Entrypoint &ep, const char *label)
	:
		_env           (env),
		_md_alloc      (md_alloc),
		_ep            (ep),
		_parent_pd     (env, label),
		_address_space (_ep, _md_alloc, _parent_pd.address_space(), "address_space"),
		_stack_area    (_ep, _md_alloc, _parent_pd.stack_area(),    "stack_area"),
		_linker_area   (_ep, _md_alloc, _parent_pd.linker_area(),   "linker_area")
	{
		_ep.manage(*this);

		if(verbose_debug) Genode::log("Pd_session_component created");
	}

	/**
	 * Destructor
	 */
	~Pd_session_component()
	{
		_ep.dissolve(*this);

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
	 * Return Address space component
	 *
	 * \return Reference to Region_map_component of the address space
	 */
	Rtcr::Region_map_component &address_space_component()
	{
		return _address_space;
	}

	/**
	 * Return Stack area component
	 *
	 * \return Reference to Region_map_component of the stack area
	 */
	Rtcr::Region_map_component &stack_area_component()
	{
		return _stack_area;
	}

	/**
	 * Return Linker area component
	 *
	 * \return Reference to Region_map_component of the linker area
	 */
	Rtcr::Region_map_component &linker_area_component()
	{
		return _linker_area;
	}

	/**************************
	 ** Pd_session interface **
	 **************************/

	void assign_parent(Genode::Capability<Genode::Parent> parent) override
	{
		if(verbose_debug)
		{
			Genode::log("Pd::\033[33m", "assign_parent", "\033[0m(", parent,")");
		}

		_parent_pd.assign_parent(parent);
	}

	bool assign_pci(Genode::addr_t addr, Genode::uint16_t bdf) override
	{
		if(verbose_debug)
		{
			Genode::log("Pd::\033[33m", "assign_pci", "\033[0m(addr=", addr,", bdf=", bdf,")");
		}

		bool result = _parent_pd.assign_pci(addr, bdf);

		if(verbose_debug)
		{
			Genode::log("  result: ", result);
		}

		return result;
	}

	Signal_source_capability alloc_signal_source() override
	{
		if(verbose_debug)
		{
			Genode::log("Pd::\033[33m", "alloc_signal_source", "\033[0m()");
		}

		Signal_source_capability result = _parent_pd.alloc_signal_source();

		if(verbose_debug)
		{
			Genode::log("  result: ", result);
		}

		return result;
	}

	void free_signal_source(Signal_source_capability cap) override
	{
		if(verbose_debug)
		{
			Genode::log("Pd::\033[33m", "free_signal_source", "\033[0m(", cap, ")");
		}

		_parent_pd.free_signal_source(cap);
	}

	Genode::Capability<Genode::Signal_context> alloc_context(Signal_source_capability source,
			unsigned long imprint) override
	{
		if(verbose_debug)
		{
			Genode::log("Pd::\033[33m", "alloc_context", "\033[0m(source ", source, ", imprint=", Genode::Hex(imprint), ")");
		}

		Genode::Capability<Genode::Signal_context> result = _parent_pd.alloc_context(source, imprint);

		if(verbose_debug)
		{
			Genode::log("  result: ", result);
		}

		return result;
	}

	void free_context(Genode::Capability<Genode::Signal_context> cap) override
	{
		if(verbose_debug)
		{
			Genode::log("Pd::\033[33m", "free_context", "\033[0m(", cap, ")");
		}

		_parent_pd.free_context(cap);
	}

	void submit(Genode::Capability<Genode::Signal_context> context, unsigned cnt) override
	{
		if(verbose_debug)
		{
			Genode::log("Pd::\033[33m", "submit", "\033[0m(context ", context, ", cnt=", cnt,")");
		}

		_parent_pd.submit(context, cnt);
	}

	Genode::Native_capability alloc_rpc_cap(Genode::Native_capability ep) override
	{
		if(verbose_debug)
		{
			Genode::log("Pd::\033[33m", "alloc_rpc_cap", "\033[0m(", ep, ")");
		}

		Genode::Native_capability result = _parent_pd.alloc_rpc_cap(ep);

		if(verbose_debug)
		{
			Genode::log("  result: ", result);
		}

		return result;
	}

	void free_rpc_cap(Genode::Native_capability cap) override
	{
		if(verbose_debug)
		{
			Genode::log("Pd::\033[33m", "free_rpc_cap", "\033[0m(", cap,")");
		}

		_parent_pd.free_rpc_cap(cap);
	}

	/**
	 * Return custom address space
	 */
	Genode::Capability<Genode::Region_map> address_space() override
	{
		if(verbose_debug)
		{
			Genode::log("Pd::\033[33m", "address_space", "\033[0m()");
		}

		Genode::Capability<Genode::Region_map> result = _address_space.Rpc_object<Genode::Region_map>::cap();

		if(verbose_debug)
		{
			Genode::log("  result: ", result);
		}

		return result;
	}

	/**
	 * Return custom stack area
	 */
	Genode::Capability<Genode::Region_map> stack_area() override
	{
		if(verbose_debug)
		{
			Genode::log("Pd::\033[33m", "stack_area", "\033[0m()");
		}

		Genode::Capability<Genode::Region_map> result = _stack_area.Rpc_object<Genode::Region_map>::cap();

		if(verbose_debug)
		{
			Genode::log("  result: ", result);
		}

		return result;
	}

	/**
	 * Return custom linker area
	 */
	Genode::Capability<Genode::Region_map> linker_area() override
	{
		if(verbose_debug)
		{
			Genode::log("Pd::\033[33m", "linker_area", "\033[0m()");
		}

		Genode::Capability<Genode::Region_map> result = _linker_area.Rpc_object<Genode::Region_map>::cap();

		if(verbose_debug)
		{
			Genode::log("  result: ", result);
		}

		return result;
	}

	Genode::Capability<Native_pd> native_pd() override
	{
		if(verbose_debug)
		{
			Genode::log("Pd::\033[33m", "native_pd", "\033[0m()");
		}

		Genode::Capability<Native_pd> result = _parent_pd.native_pd();

		if(verbose_debug)
		{
			Genode::log("  result: ", result);
		}

		return result;
	}

};

#endif /* _RTCR_PD_SESSION_COMPONENT_H_ */
