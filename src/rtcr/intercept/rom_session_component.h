/*
 * \brief  Intercepting ROM session
 * \author Denis Huber
 * \date   2016-10-24
 */

#ifndef _RTCR_ROM_SESSION_COMPONENT_H_
#define _RTCR_ROM_SESSION_COMPONENT_H_

#include <rom_session/connection.h>
#include <base/rpc_server.h>
#include <base/entrypoint.h>

namespace Rtcr {
	class Rom_session_component;

	constexpr bool rom_session_verbose_debug = true;
}

class Rtcr::Rom_session_component : public Genode::Rpc_object<Genode::Rom_session>
{
private:
	/**
	 * Enable log output for debugging
	 */
	static constexpr bool verbose_debug = rom_session_verbose_debug;

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
	Genode::Entrypoint     &_ep;
	/**
	 * Connection to parent's ROM session
	 */
	Genode::Rom_connection  _parent_rom;
	/**
	 * Parent's session state
	 */
	struct State_info
	{
		Genode::Signal_context_capability sigh { };
	} _parent_state;

public:
	Rom_session_component(Genode::Env &env, Genode::Allocator &md_alloc, Genode::Entrypoint &ep, const char *label);
	~Rom_session_component();

	State_info                     parent_state() { return _parent_state; }
	Genode::Rom_session_capability parent_cap()   { return _parent_rom;   }

	Genode::Rom_dataspace_capability dataspace() override;
	bool update() override;
	void sigh(Genode::Signal_context_capability sigh) override;
};

#endif /* _RTCR_ROM_SESSION_COMPONENT_H_ */
