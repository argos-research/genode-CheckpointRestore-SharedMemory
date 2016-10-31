/*
 * \brief  Test program for Rtcr
 * \author Denis Huber
 * \date   2016-10-09
 */

#include <base/log.h>
#include <base/component.h>
#include <timer_session/connection.h>
#include <rm_session/connection.h>
#include <base/internal/cap_map.h>
#include <dataspace/client.h>

void test_entrypoint_creation(Genode::Env &env)
{
	Genode::log("Creating Entrypoint");

	Genode::Entrypoint ep(env, 12*1024, "test_ep");

	Genode::Entrypoint ep2(env, 12*1024, "test_ep2");
}

void test_signal_context_creation(Genode::Env &env)
{
	Genode::log("Creating signal context and receiver");

	Genode::Signal_context context;
	Genode::Signal_receiver receiver;
	receiver.manage(&context);
}

void test_dataspace_creation(Genode::Env &env)
{
	Genode::log("Creating a dataspace");

	env.ram().alloc(4096);
}

Genode::size_t Component::stack_size() { return 16*1024; }

void Component::construct(Genode::Env &env)
{
	Genode::log("Hello World!");
	//Genode::log("Capability map in: ", Genode::cap_map());
	//Genode::log("Cap of RAM session: ", env.ram_session_cap(), " ", Genode::Hex(env.ram_session_cap().data()->kcap()));

	//test_entrypoint_creation(env);

	//Genode::log(Genode::Hex(Genode::Dataspace_client(env.rm().dataspace()).size()));

	Genode::log("I'm done!");
}
