/*
 * \brief  Intercepting Pd session
 * \author Denis Huber
 * \date   2016-08-03
 */

#ifndef _RTCR_PD_SESSION_H_
#define _RTCR_PD_SESSION_H_

/* Genode includes */
#include <root/component.h>
#include <base/allocator.h>
#include <base/rpc_server.h>
#include <pd_session/connection.h>

/* Rtcr includes */
#include "region_map_component.h"
#include "../monitor/pd_session_info.h"

namespace Rtcr {
	class Pd_session_component;
	class Pd_root;

	constexpr bool pd_verbose_debug = true;
	constexpr bool pd_root_verbose_debug = false;
}

/**
 * Custom RPC session object to intercept its creation, modification, and destruction through its interface
 */
class Rtcr::Pd_session_component : public Genode::Rpc_object<Genode::Pd_session>,
                                   public Genode::List<Pd_session_component>::Element
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
	 * Reference to Target_child's bootstrap phase
	 */
	bool                  &_bootstrap_phase;
	/**
	 * Connection to parent's pd session, usually from core
	 */
	Genode::Pd_connection  _parent_pd;
	/**
	 * State of parent's RPC object
	 */
	Pd_session_info        _parent_state;

	/**
	 * Custom address space for monitoring the attachments of the Region map
	 */
	Region_map_component   _address_space;
	/**
	 * Custom stack area for monitoring the attachments of the Region map
	 */
	Region_map_component   _stack_area;
	/**
	 * Custom linker area for monitoring the attachments of the Region map
	 */
	Region_map_component   _linker_area;



public:
	Pd_session_component(Genode::Env &env, Genode::Allocator &md_alloc, Genode::Entrypoint &ep,
			const char *label, const char *creation_args, bool &bootstrap_phase);
	~Pd_session_component();

	Genode::Pd_session_capability parent_cap() { return _parent_pd.cap(); }

	Region_map_component &address_space_component() { return _address_space; }
	Region_map_component const &address_space_component() const { return _address_space; }

	Region_map_component &stack_area_component() { return _stack_area; }
	Region_map_component const &stack_area_component() const { return _stack_area; }

	Region_map_component &linker_area_component() { return _linker_area; }
	Region_map_component const &linker_area_component() const { return _linker_area; }

	Pd_session_info &parent_state() { return _parent_state; }
	Pd_session_info const &parent_state() const { return _parent_state;}

	Pd_session_component *find_by_badge(Genode::uint16_t badge);

	/**************************
	 ** Pd_session interface **
	 **************************/

	void assign_parent(Genode::Capability<Genode::Parent> parent) override;

	bool assign_pci(Genode::addr_t addr, Genode::uint16_t bdf) override;

	Signal_source_capability alloc_signal_source() override;
	void free_signal_source(Signal_source_capability cap) override;

	Genode::Signal_context_capability alloc_context(Signal_source_capability source,
			unsigned long imprint) override;
	void free_context(Genode::Signal_context_capability cap) override;

	void submit(Genode::Signal_context_capability context, unsigned cnt) override;

	Genode::Native_capability alloc_rpc_cap(Genode::Native_capability ep) override;
	void free_rpc_cap(Genode::Native_capability cap) override;
	/**
	 * Return custom address space
	 */
	Genode::Capability<Genode::Region_map> address_space() override;
	/**
	 * Return custom stack area
	 */
	Genode::Capability<Genode::Region_map> stack_area() override;
	/**
	 * Return custom linker area
	 */
	Genode::Capability<Genode::Region_map> linker_area() override;
	Genode::Capability<Native_pd> native_pd() override;

};


/**
 * Custom root RPC object to intercept session RPC object creation, modification, and destruction through the root interface
 */
class Rtcr::Pd_root : public Genode::Root_component<Pd_session_component>
{
private:
	/**
	 * Enable log output for debugging
	 */
	static constexpr bool verbose_debug = pd_root_verbose_debug;

	/**
	 * Environment of Rtcr; is forwarded to a created session object
	 */
	Genode::Env        &_env;
	/**
	 * Allocator for session objects and monitoring list elements
	 */
	Genode::Allocator  &_md_alloc;
	/**
	 * Entrypoint for managing session objects
	 */
	Genode::Entrypoint &_ep;
	/**
	 * Reference to Target_child's bootstrap phase
	 */
	bool               &_bootstrap_phase;
	/**
	 * Lock for infos list
	 */
	Genode::Lock        _objs_lock;
	/**
	 * List for monitoring session objects
	 */
	Genode::List<Pd_session_component> _session_rpc_objs;

protected:
	Pd_session_component *_create_session(const char *args);
	void _upgrade_session(Pd_session_component *session, const char *upgrade_args);
	void _destroy_session(Pd_session_component *session);

public:
	Pd_root(Genode::Env &env, Genode::Allocator &md_alloc, Genode::Entrypoint &session_ep,
			bool &bootstrap_phase);
    ~Pd_root();

	Genode::List<Pd_session_component> &session_infos() { return _session_rpc_objs; }
};

#endif /* _RTCR_PD_SESSION_H_ */
