/*
 * \brief  Intercepting Rm session
 * \author Denis Huber
 * \date   2016-10-02
 */

#ifndef _RTCR_RM_SESSION_H_
#define _RTCR_RM_SESSION_H_

/* Genode includes */
#include <rm_session/connection.h>
#include <root/component.h>
#include <util/list.h>

/* Rtcr includes */
#include "../monitor/rm_session_info.h"

namespace Rtcr {
	class Rm_session_component;
	class Rm_root;

	constexpr bool rm_verbose_debug = true;
	constexpr bool rm_root_verbose_debug = true;
}

/**
 * Custom RPC session object to intercept its creation, modification, and destruction through its interface
 */
class Rtcr::Rm_session_component : public Genode::Rpc_object<Genode::Rm_session>,
                                   public Genode::List<Rm_session_component>::Element
{
private:
	/**
	 * Enable log output for debugging
	 */
	static constexpr bool verbose_debug = rm_verbose_debug;

	/**
	 * Allocator for Rpc objects created by this session and also for monitoring structures
	 */
	Genode::Allocator     &_md_alloc;
	/**
	 * Entrypoint for managing created Rpc objects
	 */
	Genode::Entrypoint    &_ep;
	/**
	 * Reference to Target_child's bootstrap phase
	 */
	bool                  &_bootstrap_phase;
	/**
	 * Parent's session connection which is used by the intercepted methods
	 */
	Genode::Rm_connection  _parent_rm;
	/**
	 * State of parent's RPC object
	 */
	Rm_session_info        _parent_state;


	Region_map_component &_create(Genode::size_t size);
	void _destroy(Region_map_component &region_map);

public:
	Rm_session_component(Genode::Env &env, Genode::Allocator &md_alloc, Genode::Entrypoint &ep,
			const char *creation_args, bool &bootstrap_phase);
	~Rm_session_component();

	Rm_session_info &parent_state() { return _parent_state; }

	/******************************
	 ** Rm session Rpc interface **
	 ******************************/

	/**
	 * Create a virtual Region map, its real counter part and a list element to manage them
	 */
	Genode::Capability<Genode::Region_map> create(Genode::size_t size) override;
	/**
	 * Destroying the virtual Region map, its real counter part, and the list element it was managed in
	 *
	 * XXX Untested! During the implementation time, the destroy method was not working.
	 */
	void destroy(Genode::Capability<Genode::Region_map> region_map_cap) override;
};

/**
 * Custom root RPC object to intercept session RPC object creation, modification, and destruction through the root interface
 */
class Rtcr::Rm_root : public Genode::Root_component<Rm_session_component>
{
private:
	/**
	 * Enable log output for debugging
	 */
	static constexpr bool verbose_debug = rm_root_verbose_debug;

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
	Genode::List<Rm_session_component> _session_rpc_objs;

protected:
	Rm_session_component *_create_session(const char *args);
	void _destroy_session(Rm_session_component *session);

public:
	Rm_root(Genode::Env &env, Genode::Allocator &md_alloc, Genode::Entrypoint &session_ep, bool &bootstrap_phase);
    ~Rm_root();

	Genode::List<Rm_session_component> &session_infos() { return _session_rpc_objs; }
};

#endif /* _RTCR_RM_SESSION_H_ */
