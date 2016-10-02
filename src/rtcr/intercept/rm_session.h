/*
 * \brief  Intercepting Rm session
 * \author Denis Huber
 * \date   2016-10-02
 */

#ifndef _RTCR_RM_SESSION_H_
#define _RTCR_RM_SESSION_H_

/* Genode includes */
#include <rm_session/rm_session.h>
#include <root/component.h>
#include <util/list.h>

/* Rtcr includes */
#include "region_map_component.h"

namespace Rtcr {
	struct Region_map_info;
	class Rm_session_component;
	class Rm_root;

	constexpr bool rm_verbose_debug = false;
}

struct Rtcr::Region_map_info : Genode::List<Region_map_info>::Element
{

};

class Rtcr::Rm_session_component : public Genode::Rm_session
{
private:
	/**
	 * Enable log output for debugging
	 */
	static constexpr bool verbose_debug = rm_verbose_debug;

	Genode::Allocator     &_md_alloc;
	Genode::Entrypoint    &_ep;
	Genode::Rm_connection  _parent_rm;

public:
	Rm_session_component(Genode::Env &env, Genode::Allocator &md_alloc, Genode::Entrypoint &ep)
	:
		_md_alloc  (md_alloc),
		_ep        (ep),
		_parent_rm (env)
	{ }

	Genode::Capability<Genode::Region_map> create(Genode::size_t size)
	{
		if(verbose_debug) Genode::log("Rm::\033[33m", "create", "\033[0m(size=", size, ")");

		auto parent_region_map_cap = _parent_rm.create(size);

		Region_map_component new_region_map =
				new (_md_alloc) Region_map_component(_ep, _md_alloc, parent_region_map_cap, "custom");

		if(verbose_debug) Genode::log("  result: ", new_region_map.cap());

		return new_region_map.cap();
	}

	void destroy(Genode::Capability<Genode::Region_map> region_map_cap)
	{
		if(verbose_debug) Genode::log("Rm::\033[33m", "destroy", "\033[0m(", region_map_cap, ")");

		_parent_rm.destroy(region_map_cap);
	}
};

class Rtcr::Rm_root : public Genode::Root_component<Rm_root>
{
private:
	Genode::Env        &_env;
	Genode::Allocator  &_md_alloc;
	Genode::Entrypoint &_ep;

protected:
	Rm_session_component *_create_session(const char *args)
	{
		return new (md_alloc()) Rm_session_component(_env, _md_alloc, _ep);
	}

public:
	Rm_root(Genode::Env &env, Genode::Allocator &md_alloc, Genode::Entrypoint &session_ep)
	:
		Root_component<Rm_root>(session_ep, md_alloc),
		_env      (env),
		_md_alloc (md_alloc),
		_ep       (session_ep)
	{ }
};

#endif /* _RTCR_RM_SESSION_H_ */
