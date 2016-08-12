/*
 * \brief Cpu root interface
 * \author Denis Huber
 * \date 2016-08-10
 */

#ifndef _RTCR_CPU_ROOT_H_
#define _RTCR_CPU_ROOT_H_

/* Genode includes */
#include <root/component.h>

/* Rtcr includes */
#include "cpu_session_component.h"

namespace Rtcr {
	class Cpu_root;
	using namespace Genode;
}

class Rtcr::Cpu_root : public Root_component<Rtcr::Cpu_session_component>
{
private:
	static constexpr bool verbose = true;
	/**
	 * Checkpointer's environment to issue a session request to parent's
	 * Cpu service (usually forwards it to core)
	 */
	Env &_env;
	/**
	 * Parent Pd cap for creating threads using parents pd cap (usually core's)
	 */
	Pd_session_capability _parent_pd_cap;

protected:

	Rtcr::Cpu_session_component *_create_session(const char *args)
	{
		return new (md_alloc())
				Rtcr::Cpu_session_component(_env, *md_alloc(), _parent_pd_cap, args);
	}

public:

	Cpu_root(Env &env, Allocator &alloc, Pd_session_capability parent_pd_cap)
	:
		Root_component<Rtcr::Cpu_session_component>(env.ep(), alloc),
		_env(env),
		_parent_pd_cap(parent_pd_cap)
	{
		if(verbose) log("Cpu_root created");
	}
};

#endif /* _RTCR_CPU_ROOT_H_ */
