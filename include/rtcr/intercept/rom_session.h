/*
 * \brief  Intercepting ROM session
 * \author Denis Huber
 * \date   2016-10-24
 */

#ifndef _RTCR_ROM_SESSION_COMPONENT_H_
#define _RTCR_ROM_SESSION_COMPONENT_H_

/* Genode includes */
#include <root/component.h>
#include <rom_session/connection.h>
#include <dataspace/client.h>
#include <base/rpc_server.h>
#include <base/entrypoint.h>
#include <base/allocator.h>
#include <base/session_object.h>

/* Rtcr includes */
#include "../online_storage/rom_session_info.h"

namespace Rtcr {
	class Rom_session_component;
	class Rom_root;

	constexpr bool rom_verbose_debug = false;
	constexpr bool rom_root_verbose_debug = false;
}

class Rtcr::Rom_session_component : public Genode::Rpc_object<Genode::Rom_session>,
                                    private Genode::List<Rom_session_component>::Element
{
private:
	friend class Genode::List<Rtcr::Rom_session_component>;
	/**
	 * Enable log output for debugging
	 */
	static constexpr bool verbose_debug = rom_verbose_debug;

	/**
	 * Environment of creator component (usually rtcr)
	 */
	Genode::Env            &_env;
	/**
	 * Allocator
	 */
	Genode::Allocator      &_md_alloc;
	/**
	 * Entrypoint
	 */
	Genode::Rpc_entrypoint     &_ep;
	/**
	 * Connection to parent's ROM session
	 */
	Genode::Rom_connection  _parent_rom;
	/**
	 * State of parent's RPC object
	 */
	Rom_session_info        _parent_state;

public:
	Rom_session_component(Genode::Env &env, Genode::Allocator &md_alloc, Genode::Rpc_entrypoint &ep,
			const char *label, const char *creation_args, bool &bootstrap_phase);
	~Rom_session_component();

	Genode::Rom_session_capability parent_cap() { return _parent_rom.cap(); }

	Rom_session_info &parent_state() { return _parent_state; }
	Rom_session_info const &parent_state() const { return _parent_state; }

	Rom_session_component *find_by_badge(Genode::uint16_t badge);

	using Genode::List<Rtcr::Rom_session_component>::Element::next;

	/*******************************
	 ** Rom session Rpc interface **
	 *******************************/

	Genode::Rom_dataspace_capability dataspace() override;
	bool update() override;
	void sigh(Genode::Signal_context_capability sigh) override;
};

/**
 * Custom root RPC object to intercept session RPC object creation, modification, and destruction through the root interface
 */
class Rtcr::Rom_root : public Genode::Root_component<Rom_session_component>
{
private:
	/**
	 * Enable log output for debugging
	 */
	static constexpr bool verbose_debug = rom_root_verbose_debug;

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
	Genode::List<Rom_session_component> _session_rpc_objs;

public:
	Rom_session_component *_create_session(const char *args);
	void _upgrade_session(Rom_session_component *session, const char *upgrade_args);
	void _destroy_session(Rom_session_component *session);

	Rom_root(Genode::Env &env, Genode::Allocator &md_alloc, Genode::Entrypoint &session_ep, bool &bootstrap_phase);
    ~Rom_root();

	Genode::List<Rom_session_component> &session_infos() { return _session_rpc_objs; }
};

#endif /* _RTCR_ROM_SESSION_COMPONENT_H_ */
