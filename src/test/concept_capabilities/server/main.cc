/*
 * \brief  Server which manipulates the cap space of the client
 * \author Denis Huber
 * \date   2016-09-20
 */

/* Genode includes */
#include <base/env.h>
#include <base/log.h>
#include <base/sleep.h>
#include <base/component.h>
#include <base/rpc_server.h>
#include <base/heap.h>
#include <root/component.h>
#include <timer_session/connection.h>
#include <pd_session/pd_session.h>

/* Fiasco.OC includes */
#include <foc_native_pd/client.h>

/* Resource includes */
#include <resource_session/resource_session.h>

namespace Resource {
	struct Client_resources;
	class  Session_component;
	class  Root;
	struct Main;
}

struct Resource::Client_resources
{
	Genode::Capability<Genode::Pd_session::Native_pd> native_pd_cap;

	Client_resources()
	:
		native_pd_cap()
	{}
};

class Resource::Session_component : public Genode::Rpc_object<Session>
{
private:
	Client_resources &_cli_res;

public:
	Session_component(Client_resources &cli_res)
	:
		_cli_res(cli_res)
	{ }

	void provide(Genode::Native_capability native_pd_cap, Genode::uint32_t)
	{
		_cli_res.native_pd_cap = Genode::reinterpret_cap_cast<Genode::Pd_session::Native_pd>(native_pd_cap);
	}
	Genode::Native_capability request(Genode::uint32_t)
	{
		return Genode::Native_capability();
	}
};

class Resource::Root : public Genode::Root_component<Session_component>
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

struct Resource::Main
{
	Genode::Env                &env;
	Resource::Client_resources  cli_res;
	Genode::Sliced_heap         sliced_heap;
	Genode::Entrypoint          session_ep;
	Resource::Root              root;
	Timer::Connection           timer;


	Main(Genode::Env &env)
	:
		env         (env),
		cli_res     (),
		sliced_heap (env.ram(), env.rm()),
		session_ep  (env, 16*1024, "session ep"),
		root        (session_ep, sliced_heap, cli_res),
		timer       (env)
	{
		using Genode::log;

		log("Announcing Resource service");
		env.parent().announce(session_ep.manage(root));

		while(true)
		{
			if(cli_res.native_pd_cap.valid())
			{
				log("Valid pd_cap");

				Genode::Capability<Genode::Foc_native_pd> foc_pd_cap =
						Genode::static_cap_cast<Genode::Foc_native_pd>(cli_res.native_pd_cap);

				Genode::Foc_native_pd_client foc_pd_client{foc_pd_cap};
				foc_pd_client.request(1);
				foc_pd_client.install(foc_pd_cap, 1);
			}
			else
			{
				log("Invalid pd_cap");
			}

			timer.msleep(4000);
		}

		Genode::sleep_forever();
	}
};

Genode::size_t Component::stack_size() { return 64*1024; }

void Component::construct(Genode::Env &env)
{
	static Resource::Main main(env);
}
