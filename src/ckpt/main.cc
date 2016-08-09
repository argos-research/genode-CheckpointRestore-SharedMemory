/*
 * \brief Checkpointer
 * \author Denis Huber
 * \date 2016-08-04
 */

/* Genode include */
#include <base/component.h>
#include <base/signal.h>
#include <base/sleep.h>
#include <base/log.h>

/* Rtcr includes */
#include "child.h"
#include "pd_session_component.h"

namespace Rtcr {
	struct Main;
}

struct Rtcr::Main
{

	Genode::Env &env;
/*
	Genode::Sliced_heap sliced_heap { env.ram(), env.rm() };
	Rtcr::Pd_root pd_root { env, env.ep(), sliced_heap };
*/
	enum { STACK_SIZE = 8*1024 };
	Genode::Entrypoint ep { env, STACK_SIZE, "child_stack" };
	Genode::Heap md_heap { env.ram(), env.rm() };
	/**
	 * Signal_handler is a Signal_context_capability (can be targeted by
	 * Signal_transmitter) and a Signal_dispatcher_base (executes a function
	 * in the Entrypoint when a signal arrives)
	 */
	//Genode::Signal_handler<Main> sig_handler { ep, *this, &Main::handle_signal };

	Main(Genode::Env &env_) : env(env_)
	{
		//env.parent().announce(env.ep().manage(pd_root));

		Target_child child { env, md_heap, ep, ep, "sheep_counter" };

		Genode::sleep_forever();
	}

	void handle_signal() {
		Genode::log(__PRETTY_FUNCTION__); }
};

Genode::size_t Component::stack_size() { return 64*1024; }

void Component::construct(Genode::Env &env)
{
	static Rtcr::Main main(env);
}
