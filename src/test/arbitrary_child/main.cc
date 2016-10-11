/*
 * \brief  Test program for Rtcr
 * \author Denis Huber
 * \date   2016-10-09
 */

#include <base/log.h>
#include <base/component.h>
#include <timer_session/connection.h>
#include <rm_session/connection.h>

void test_entrypoint_creation(Genode::Env &env)
{
	Genode::log("Creating Entrypoint");

	Genode::Entrypoint ep(env, 12*1024, "test_ep");
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

	Genode::cap_map();

	test_signal_context_creation(env);
}
