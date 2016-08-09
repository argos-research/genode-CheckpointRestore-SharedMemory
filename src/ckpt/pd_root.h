/*
 * \brief PD root interface
 * \author Denis Huber
 * \date 2016-08-04
 */

/* Genode includes */
#include <root/component.h>

/* Rtcr includes */
#include "../intercepted/pd_session_component.h"

namespace Rtcr {
	class Pd_root;
}

// Root_component introduces session(), upgrade(), close() functions from the Root interface
class Rtcr::Pd_root : public Genode::Root_component<Rtcr::Pd_session_component>
{
private:
	Genode::Env &_env;

protected:

	Rtcr::Pd_session_component *_create_session(const char *args)
	{
		// md_alloc() is the Allocator provided through the ctor
		// This means, the creator of this Root component decides whose Allocator to use
		// Typically, the Allocator is provided by the creator
		return new (md_alloc()) Rtcr::Pd_session_component(_env);
	}

public:

	Pd_root(Genode::Env &env, Genode::Entrypoint &ep, Genode::Allocator &alloc)
	:
		Genode::Root_component<Rtcr::Pd_session_component>(ep, alloc), _env(&env)
	{ }
};
