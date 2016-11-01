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
#include "region_map_component.h"

namespace Rtcr {
	class Rm_session_component;
	class Rm_root;

	constexpr bool rm_verbose_debug = true;
	constexpr bool rm_root_verbose_debug = true;

	// Forward declaration
	struct Region_map_info;
	struct Rm_session_info;
}


class Rtcr::Rm_session_component : public Genode::Rpc_object<Genode::Rm_session>
{
private:
	/**
	 * Enable log output for debugging
	 */
	static constexpr bool verbose_debug = rm_verbose_debug;

	/**
	 * Allocator for Rpc objects created by this session and also for monitoring structures
	 */
	Genode::Allocator             &_md_alloc;
	/**
	 * Entrypoint for managing created Rpc objects
	 */
	Genode::Entrypoint            &_ep;
	/**
	 * Parent's session connection which is used by the intercepted methods
	 */
	Genode::Rm_connection          _parent_rm;
    /**
     * Lock for infos list
     */
	Genode::Lock                   _infos_lock;
    /**
     * List for monitoring Rpc object
     */
	Genode::List<Region_map_info>  _region_map_infos;

public:
	Rm_session_component(Genode::Env &env, Genode::Allocator &md_alloc, Genode::Entrypoint &ep);
	~Rm_session_component();

	Genode::List<Region_map_info> &region_map_infos()           { return _region_map_infos;  }
    void region_map_infos(Genode::List<Region_map_info> &infos) { _region_map_infos = infos; }

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
 * Virtual Root session object to intercept Rm session object creation
 * This enables the Rtcr component to monitor capabilities created for Rm session objects
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
	Genode::Env                   &_env;
	/**
	 * Allocator for session objects and monitoring list elements
	 */
	Genode::Allocator             &_md_alloc;
	/**
	 * Entrypoint for managing session objects
	 */
	Genode::Entrypoint            &_ep;
	/**
	 * Lock for infos list
	 */
	Genode::Lock                   _infos_lock;
	/**
	 * List for monitoring Rm session objects
	 */
	Genode::List<Rm_session_info>  _rms_infos;

protected:
	Rm_session_component *_create_session(const char *args);
	void _destroy_session(Rm_session_component *session);

public:
	Rm_root(Genode::Env &env, Genode::Allocator &md_alloc, Genode::Entrypoint &session_ep);
    ~Rm_root();

	Genode::List<Rm_session_info> &rms_infos() { return _rms_infos; }
    void rms_infos(Genode::List<Rm_session_info> &infos) { _rms_infos = infos; }
};

#endif /* _RTCR_RM_SESSION_H_ */
