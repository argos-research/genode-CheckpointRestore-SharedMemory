/*
 * \brief  Random program for testing Genode's functionalities
 * \author Denis Huber
 * \date   2016-08-13
 */

#include <base/log.h>
#include <base/rpc_server.h>
#include <base/component.h>
#include <ram_session/connection.h>

namespace Random {
	class My_ram_session_component;
}

class Random::My_ram_session_component : public  Genode::Rpc_object<Genode::Ram_session>
{
public:

	Genode::Ram_connection _parent_ram;

	My_ram_session_component() : _parent_ram()
	{
		Genode::log("My_ram_session_component created");
	}

	~My_ram_session_component()
	{
		Genode::log("My_ram_session_component destroyed");
	}

	Genode::Ram_dataspace_capability alloc(Genode::size_t size, Genode::Cache_attribute cached) override
	{
		return _parent_ram.alloc(size, cached);
	}

	void free(Genode::Ram_dataspace_capability ds) override
	{
		_parent_ram.free(ds);
	}

	int ref_account(Genode::Ram_session_capability ram_session) override
	{
		return _parent_ram.ref_account(ram_session);
	}

	int transfer_quota(Genode::Ram_session_capability ram_session, Genode::size_t amount) override
	{
		return _parent_ram.transfer_quota(ram_session, amount);
	}

	Genode::size_t quota() override
	{
		return _parent_ram.quota();
	}

	Genode::size_t used() override
	{
		return _parent_ram.used();
	}

};

Genode::size_t Component::stack_size() { return 32*4*1024; }

void Component::construct(Genode::Env &env)
{
	using namespace Genode;

	Entrypoint ep(env, 16*1024, "ram ep");
	Random::My_ram_session_component ram_impl;
	Ram_session_client ram_special { ep.manage(ram_impl) };

	log("r: ", ram_special.ref_account(env.ram_session_cap()));
	log("q: ", ram_special.quota());
	log("t: ", env.ram().transfer_quota(ram_special, 4096));
	log("q: ", ram_special.quota());

	log("Random ended");
}
