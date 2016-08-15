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
}

struct Rtcr::Main
{

	Genode::Env &env;

	enum { STACK_SIZE = 16*1024 };
	/**
	 * Entrypoint used to virtualize child's resources like RAM, CPU
	 */
	Genode::Entrypoint resource_ep { env, STACK_SIZE, "childs ep" };
	Genode::Heap md_heap { env.ram(), env.rm() };
	Genode::Service_registry parent_services;
	const char *label = "sheep_counter";
	/**
	 * Signal_handler is a Signal_context_capability (can be targeted by
	 * Signal_transmitter) and a Signal_dispatcher_base (executes a function
	 * in the Entrypoint when a signal arrives)
	 */
	//Signal_handler<Main> sig_handler { ep, *this, &Main::handle_signal };

	Main(Genode::Env &env_) : env(env_)
	{
		Genode::log("Main env: ", &env);
		Genode::log("before creating child");

		Target_child child { env, resource_ep, md_heap, parent_services, label };

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
