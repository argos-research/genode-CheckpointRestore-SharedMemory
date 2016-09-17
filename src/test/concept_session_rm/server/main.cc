/*
 * \brief  Checkpoint component
 * \author Denis Huber
 * \date   2016-09-17
 */

/* Genode includes */
#include <base/env.h>
#include <base/log.h>
#include <base/component.h>
#include <base/rpc_server.h>
#include <base/sleep.h>
#include <root/component.h>
#include <rm_session/connection.h>
#include <timer_session/connection.h>
#include <region_map/client.h>
#include <cpu_thread/client.h>

/* Resource includes */
#include <resource_session/resource_session.h>

namespace Resource {
	struct Client_resources;
	struct Session_component;
	struct Root;
	struct Fault_handler;
}

struct Resource::Client_resources : public Genode::List<Client_resources>::Element
{
	Genode::Thread_capability thread_cap;
	Genode::size_t region_map_size;
	Genode::Capability<Genode::Region_map> region_map_cap;
	Genode::size_t size0;
	Genode::size_t size1;
	Genode::Dataspace_capability sub_ds_cap0;
	Genode::Dataspace_capability sub_ds_cap1;
	Genode::addr_t addr0;
	Genode::addr_t addr1;
	bool attached0;
	bool attached1;

	Client_resources(Genode::Env &env)
	:
		thread_cap(),
		region_map_size(8*1024),
		region_map_cap(Genode::Rm_connection(env).create(8*1024)),
		size0(4*1024),
		size1(4*1024),
		sub_ds_cap0(env.ram().alloc(size0)),
		sub_ds_cap1(env.ram().alloc(size1)),
		addr0(0),
		addr1(4*1024),
		attached0(false),
		attached1(false)
	{ }

};

struct Resource::Session_component : public Genode::Rpc_object<Session>
{
	Client_resources &_cli_res;

	Session_component(Client_resources &cli_res)
	:
		_cli_res(cli_res)
	{ }

	void thread(Genode::Thread_capability thread_cap)
	{
		_cli_res.thread_cap = thread_cap;
	}
	Genode::Dataspace_capability dataspace(Genode::size_t size)
	{
		return Genode::Region_map_client{_cli_res.region_map_cap}.dataspace();
	}
};

struct Resource::Root : public Genode::Root_component<Session_component>
{
private:
	Client_resources &_cli_res;
protected:
	Session_component *_create_session(const char *args)
	{
		return new (md_alloc()) Session_component(_cli_res);
	}

public:
	Root(Genode::Entrypoint &session_ep, Genode::Allocator &md_alloc, Client_resources &cli_res)
	:
		Root_component<Session_component>(session_ep, md_alloc), _cli_res(cli_res)
	{ }
};

struct Resource::Fault_handler : public Genode::Thread
{
	Genode::Signal_receiver &receiver;
	Genode::Signal_context  &context;
	Client_resources &cli_res;

	Fault_handler(Genode::Env &env, Client_resources &cli_res, Genode::Signal_receiver &receiver, Genode::Signal_context &context)
	:
		Thread(env, "my page fault handler", 16*1024),
		receiver(receiver), context(context),
		cli_res(cli_res)
	{ }

	void handle_fault()
	{
		Genode::Region_map_client rm_client {cli_res.region_map_cap};
		Genode::Region_map::State state = rm_client.state();

		if(state.addr < 4096)
		{
			rm_client.attach_at(cli_res.sub_ds_cap0, cli_res.addr0);
			cli_res.attached0 = true;
		}
		else
		{
			rm_client.attach_at(cli_res.sub_ds_cap1, cli_res.addr1);
			cli_res.attached1 = true;
		}
	}

	void entry()
	{
		while(true)
		{
			Genode::Signal signal = receiver.wait_for_signal();
			for(unsigned int i = 0; i < signal.num(); ++i)
				handle_fault();
		}
	}
};

Genode::size_t Component::stack_size() { return 64*1024; }

void Component::construct(Genode::Env &env)
{
	using namespace Genode;
	using namespace Resource;

	Client_resources cli_res{env};
	Signal_receiver receiver;
	Signal_context context;
	//Genode::Sliced_heap sliced_heap {env.ram(), env.rm()};
	//Root root {env.ep(), sliced_heap, cli_res};
	Fault_handler fault_handler {env, cli_res, receiver, context};

	log("Hello World");

	fault_handler.start();
	log("Step 1");
	Capability<Region_map> new_rm_cap = Rm_connection{env}.create(2*4096);
	Region_map_client new_rm_client {new_rm_cap};
	new_rm_client.fault_handler(receiver.manage(&context));
	//env.rm().fault_handler(fault_handler.receiver.manage(&fault_handler.context));
	//Region_map_client{cli_res.region_map_cap}.fault_handler(fault_handler.receiver.manage(&fault_handler.context));
	log("Step 2");
/*
	env.parent().announce(env.ep().manage(root));
	log("Step 3");

	Timer::Connection timer {env};
	log("Step 4");
	timer.msleep(2000);
	log("Step 5");

	while(true)
	{
		Cpu_thread_client{cli_res.thread_cap}.pause();

		Region_map_client{cli_res.region_map_cap}.detach(cli_res.addr0);
		cli_res.attached0 = false;
		Region_map_client{cli_res.region_map_cap}.detach(cli_res.addr1);
		cli_res.attached1 = false;

		Cpu_thread_client{cli_res.thread_cap}.resume();

		timer.msleep(4000);
	}*/

	sleep_forever();
}
