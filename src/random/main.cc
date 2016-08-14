/*
 * \brief  Random program for testing Genode's functionalities
 * \author Denis Huber
 * \date   2016-08-13
 */

#include <base/log.h>
#include <base/component.h>
#include <base/rpc_server.h>
#include <base/rpc_client.h>
#include <ram_session/connection.h>

namespace Random {
	class My_ram_session_component;
	class My_ram_session;
	class My_ram_session_client;
}

class Random::My_ram_session //: public Genode::Session
{
public:
	//static const char *service_name() { return "MYRAM"; }
	virtual ~My_ram_session() { }
	virtual int alloc() = 0;
	virtual void free(int) = 0;

	GENODE_RPC(Rpc_alloc, int, alloc);
	GENODE_RPC(Rpc_free, void, free, int);
	GENODE_RPC_INTERFACE(Rpc_alloc, Rpc_free);
};

class Random::My_ram_session_client : Genode::Rpc_client<My_ram_session>
{
public:
	explicit My_ram_session_client(Genode::Capability<My_ram_session> session)
	: Rpc_client<My_ram_session>(session) { Genode::log("My_ram_session_client created"); }

	int alloc() override
	{
		Genode::log("calling alloc()");
		return call<Rpc_alloc>();
	}

	void free(int i) override
	{
		Genode::log("calling free(", i, ")");
		call<Rpc_free>(i);
	}
};


class Random::My_ram_session_component : public  Genode::Rpc_object<My_ram_session>
{
public:

	My_ram_session_component()
	{
		Genode::log("My_ram_session_component created");
	}

	~My_ram_session_component()
	{
		Genode::log("My_ram_session_component destroyed");
	}

	int alloc() override
	{
		Genode::log("executing alloc()");
		return 42;
	}

	void free(int i) override
	{
		Genode::log("executing free(", i,")");
	}

};

Genode::size_t Component::stack_size() { return 32*4*1024; }

void Component::construct(Genode::Env &env)
{
	using namespace Genode;

	Ram_connection ram_standard;
	log("Standard ram");
	log("q:   ", ram_standard.quota());
	log("ref: ", ram_standard.ref_account(env.ram_session_cap()));
	log("q:   ", ram_standard.quota());
	log("tq:  ", env.ram().transfer_quota(ram_standard, 4*1024));
	log("q:   ", ram_standard.quota());

	Entrypoint new_ep(env, 16*1024, "new ep");
	Random::My_ram_session_component ram_impl;
	Random::My_ram_session_client ram_special { new_ep.manage(ram_impl) };
	//char buf[] = "ram_quota=16384";
	//Ram_session_client ram_special { env.parent().session<Ram_session>(buf) };
	log("Special ram");
	log("a: ", ram_special.alloc());
	ram_special.free(42);

	log("Random ended");
}
