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
	struct Signal_source_info;
	struct Signal_context_info;
	struct Native_capability_info;
	class Pd_session_component;

	constexpr bool pd_verbose_debug = true;
}

/**
 * List element to store Signal_source_capabilities created by the pd session
 */
struct Rtcr::Signal_source_info : Genode::List<Signal_source_info>::Element
{
	Genode::Capability<Genode::Signal_source> cap;

	Signal_source_info(Genode::Capability<Genode::Signal_source> cap)
	:
		cap(cap)
	{ }

	Signal_source_info *find_by_cap(Genode::Capability<Genode::Signal_source> cap)
	{
		if(cap == this->cap)
			return this;
		Signal_source_info *info = next();
		return info ? info->find_by_cap(cap) : 0;
	}
};

/**
 * List element to store Signal_context_capabilities created by the pd session
 */
struct Rtcr::Signal_context_info : Genode::List<Signal_context_info>::Element
{
	Genode::Signal_context_capability sc_cap;
	Genode::Capability<Genode::Signal_source>  ss_cap;
	/**
	 * imprint is an opaque number (Source: pd_session/pd_session.h),
	 * which is associated with the pointer of a Signal_context in Signal_receiver::manage
	 *
	 * It is sent with each signal.
	 * The usage of imprint is also opaque. It could be used as an process-unique identifier.
	 * The pointer is valid in the process which uses this virtual pd session.
	 *
	 * This means, when restoring the address space and this imprint value. It shall eventually
	 * point to the Signal context used to create this Signal_context_capability
	 */
	unsigned long                     imprint;

	Signal_context_info(Genode::Signal_context_capability sc_cap,
			Genode::Capability<Genode::Signal_source> ss_cap, unsigned long imprint)
	:
		sc_cap(sc_cap),
		ss_cap(ss_cap),
		imprint(imprint)
	{ }


	Signal_context_info *find_by_sc_cap(Genode::Signal_context_capability cap)
	{
		if(cap == this->sc_cap)
			return this;
		Signal_context_info *info = next();
		return info ? info->find_by_sc_cap(cap) : 0;
	}
};

/**
 * List element to store a capability which is created by the pd session
 * They are usually created by client's entrypoint and therefore require
 * a cpu_thread_capability
 */
struct Rtcr::Native_capability_info : Genode::List<Native_capability_info>::Element
{
	Genode::Native_capability native_cap;
	Genode::Native_capability ep_cap;

	Native_capability_info(Genode::Native_capability native_cap, Genode::Native_capability ep_cap)
	:
		native_cap(native_cap),
		ep_cap(ep_cap)
	{ }

	Native_capability_info *find_by_native_cap(Genode::Native_capability cap)
	{
		if(cap == this->native_cap)
			return this;
		Native_capability_info *info = next();
		return info ? info->find_by_native_cap(cap) : 0;
	}
};

/**
 * This virtual Pd session provides virtual Region maps
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
	Genode::Env           &_env;
	/**
	 * Allocator for list elements which monitor the Signal_source,
	 * Signal_context and Native_capability creation and destruction
	 */
	Genode::Allocator     &_md_alloc;
	/**
	 * Entrypoint to manage itself
	 */
	Genode::Entrypoint    &_ep;
	/**
	 * Connection to parent's pd session, usually from core
	 */
	Genode::Pd_connection  _parent_pd;
	/**
	 * Virtual address space for monitoring the attachments of the Region map
	 */
	Region_map_component   _address_space;
	/**
	 * Virtual stack area for monitoring the attachments of the Region map
	 */
	Region_map_component   _stack_area;
	/**
	 * Virtual linker area for monitoring the attachments of the Region map
	 */
	Region_map_component   _linker_area;

	/**
	 * Lock for Signal_source_info list
	 */
	Genode::Lock                         _ss_infos_lock;
	/**
	 * List for monitoring the creation and destruction of Signal_source_capabilities
	 */
	Genode::List<Signal_source_info>     _ss_infos;
	/**
	 * Lock for Signal_context_info list
	 */
	Genode::Lock                         _sc_infos_lock;
	/**
	 * List for monitoring the creation and destruction of Signal_context_capabilities
	 */
	Genode::List<Signal_context_info>    _sc_infos;
	/**
	 * Lock for Native_capability_info list
	 */
	Genode::Lock                         _nc_infos_lock;
	/**
	 * List for monitoring the creation and destruction of Native_capabilities
	 */
	Genode::List<Native_capability_info> _nc_infos;

public:
	/**
	 * Constructor
	 */
	Pd_session_component(Genode::Env &env, Genode::Allocator &md_alloc, Genode::Entrypoint &ep, const char *label)
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
		if(verbose_debug) Genode::log("\033[33m", __func__, "\033[0m");
	}

	~Pd_session_component()
	{
		if(verbose_debug) Genode::log("\033[33m", __func__, "\033[0m");
	}

	Genode::Pd_session_capability         parent_cap()              { return _parent_pd.cap(); }
	Region_map_component                 &address_space_component() { return _address_space;   }
	Region_map_component                 &stack_area_component()    { return _stack_area;      }
	Region_map_component                 &linker_area_component()   { return _linker_area;     }
	Genode::List<Signal_source_info>     &signal_source_infos()     { return _ss_infos;        }
	Genode::List<Signal_context_info>    &signal_context_infos()    { return _sc_infos;        }
	Genode::List<Native_capability_info> &native_capability_infos() { return _nc_infos;        }

	/**************************
	 ** Pd_session interface **
	 **************************/

	void assign_parent(Genode::Capability<Genode::Parent> parent) override
	{
		if(verbose_debug) Genode::log("Pd::\033[33m", __func__, "\033[0m(", parent,")");

		_parent_pd.assign_parent(parent);
	}

	bool assign_pci(Genode::addr_t addr, Genode::uint16_t bdf) override
	{
		if(verbose_debug) Genode::log("Pd::\033[33m", __func__, "\033[0m(addr=", addr,", bdf=", bdf,")");

		auto result = _parent_pd.assign_pci(addr, bdf);

		if(verbose_debug) Genode::log("  result: ", result);

		return result;
	}

	Signal_source_capability alloc_signal_source() override
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

	void free_signal_source(Signal_source_capability cap) override
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

	Genode::Signal_context_capability alloc_context(Signal_source_capability source,
			unsigned long imprint) override
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

	void free_context(Genode::Signal_context_capability cap) override
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

	void submit(Genode::Signal_context_capability context, unsigned cnt) override
	{
		if(verbose_debug) Genode::log("Pd::\033[33m", __func__, "\033[0m(context ", context, ", cnt=", cnt,")");

		_parent_pd.submit(context, cnt);
	}

	Genode::Native_capability alloc_rpc_cap(Genode::Native_capability ep) override
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

	void free_rpc_cap(Genode::Native_capability cap) override
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

	/**
	 * Return custom address space
	 */
	Genode::Capability<Genode::Region_map> address_space() override
	{
		if(verbose_debug) Genode::log("Pd::\033[33m", __func__, "\033[0m()");

		auto result = _address_space.Rpc_object<Genode::Region_map>::cap();

		if(verbose_debug) Genode::log("  result: ", result);

		return result;
	}

	/**
	 * Return custom stack area
	 */
	Genode::Capability<Genode::Region_map> stack_area() override
	{
		if(verbose_debug) Genode::log("Pd::\033[33m", __func__, "\033[0m()");

		auto result = _stack_area.Rpc_object<Genode::Region_map>::cap();

		if(verbose_debug) Genode::log("  result: ", result);

		return result;
	}

	/**
	 * Return custom linker area
	 */
	Genode::Capability<Genode::Region_map> linker_area() override
	{
		if(verbose_debug) Genode::log("Pd::\033[33m", __func__, "\033[0m()");

		auto result = _linker_area.Rpc_object<Genode::Region_map>::cap();

		if(verbose_debug) Genode::log("  result: ", result);

		return result;
	}

	Genode::Capability<Native_pd> native_pd() override
	{
		if(verbose_debug) Genode::log("Pd::\033[33m", __func__, "\033[0m()");

		auto result = _parent_pd.native_pd();

		if(verbose_debug) Genode::log("  result: ", result);

		return result;
	}

};

#endif /* _RTCR_PD_SESSION_COMPONENT_H_ */
