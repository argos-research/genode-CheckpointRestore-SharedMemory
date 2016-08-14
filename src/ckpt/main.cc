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
#include "target_child.h"

namespace Rtcr {
	struct Main;
	using namespace Genode;
}

struct Rtcr::Main
{

	Env &env;

	enum { STACK_SIZE = 16*1024 };
	Entrypoint child_ep { env, STACK_SIZE, "childs ep" };
	Heap md_heap { env.ram(), env.rm() };
	/**
	 * Signal_handler is a Signal_context_capability (can be targeted by
	 * Signal_transmitter) and a Signal_dispatcher_base (executes a function
	 * in the Entrypoint when a signal arrives)
	 */
	//Signal_handler<Main> sig_handler { ep, *this, &Main::handle_signal };

	Main(Env &env_) : env(env_)
	{
		log("Main env: ", &env);

		log("before creating child");
		const char *label = "sheep_counter";

		Target_child child { env, child_ep, md_heap, label };

		sleep_forever();
	}

	void handle_signal() {
		log(__PRETTY_FUNCTION__); }
};

Genode::size_t Component::stack_size() { return 64*1024; }

void Component::construct(Genode::Env &env)
{
	static Rtcr::Main main(env);
}
