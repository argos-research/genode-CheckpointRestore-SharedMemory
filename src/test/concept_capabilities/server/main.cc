/*
 * \brief  Template test
 * \author Denis Huber
 * \date   2016-08-20
 */

/* Genode includes */
#include <base/env.h>
#include <base/log.h>
#include <base/component.h>
#include <root/component.h>
#include <base/rpc_server.h>
#include <timer_session/connection.h>
#include <resource_session/resource_session.h>

namespace Resource {
	struct Client_resources;
	class Session_component;
	class Root;
}

struct Resource::Client_resources
{
	Genode::Pd_session_capability pd_cap;

	Client_resources()
	:
		pd_cap()
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

	void provide(Genode::Native_capability pd_cap, Genode::uint32_t)
	{
		_cli_res.pd_cap = Genode::reinterpret_cap_cast<Genode::Pd_session>(pd_cap);
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

struct Main
{
	Genode::Env                &env;
	Resource::Client_resources  cli_res;
	Genode::Sliced_heap         sliced_heap;
	Genode::Entrypoint          session_ep;
	Resource::Root              root;

	Main(Genode::Env &env)
	:
		env(env),
		cli_res(),
		sliced_heap(env.ram(), env.rm()),
		session_ep(env, 16*1024, "session ep"),
		root(session_ep, sliced_heap, cli_res)
	{
		using Genode::log;

		log("Announcing Resource service");
		env.parent().announce(session_ep.manage(root));
	}
};

Genode::size_t Component::stack_size() { return 64*1024; }

void Component::construct(Genode::Env &env)
{
	static Main main(env);
}
