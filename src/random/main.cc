/*
 * \brief  Random program for testing Genode's functionalities
 * \author Denis Huber
 * \date   2016-08-13
 */

#include <base/log.h>
#include <base/rpc_server.h>
#include <base/component.h>
#include <base/child.h>
#include <base/sleep.h>
#include <ram_session/connection.h>
#include <pd_session/connection.h>
#include <cpu_session/connection.h>
#include <rom_session/connection.h>

using namespace Genode;

class My_ram_session_component : public Rpc_object<Ram_session>
{
public:
	Ram_connection _parent_ram;

	My_ram_session_component() : _parent_ram()
	{ }

	~My_ram_session_component()
	{ }

	Ram_dataspace_capability alloc(size_t size, Cache_attribute cached) override
	{
		return _parent_ram.alloc(size, cached);
	}

	void free(Ram_dataspace_capability ds) override
	{
		_parent_ram.free(ds);
	}

	int ref_account(Ram_session_capability ram_session) override
	{
		return _parent_ram.ref_account(ram_session);
	}

	int transfer_quota(Ram_session_capability ram_session, size_t amount) override
	{
		return _parent_ram.transfer_quota(ram_session, amount);
	}

	size_t quota() override
	{
		return _parent_ram.quota();
	}

	size_t used() override
	{
		return _parent_ram.used();
	}
};

class My_child : public Child_policy
{
private:
	const char *_name;
	Env &_env;
	Entrypoint _child_ep;
	Pd_connection  _pd;
	Cpu_connection _cpu;
	Rom_connection _rom;
	struct Ram_manager {
		Ram_connection ram;
		Ram_manager(Env &env, const char *name) : ram(env, name)
		{
			ram.ref_account(env.ram_session_cap());
			log("env: ", env.ram().avail(), ", child: ", ram.avail());
			env.ram().transfer_quota(ram, 1024*1024);
			log("env: ", env.ram().avail(), ", child: ", ram.avail());
		}
	} _ram_manager;
	Child::Initial_thread _initial_thread;
	Region_map_client _address_space;
	Parent_service _log_service;
	Parent_service _timer_service;
	Child _child;
public:
	My_child(Env &env, const char *name)
	:
		_name(name), _env(env),
		_child_ep(env, 64*1024, "child_ep"),
		_pd(env, name), _cpu(env, name), _rom(env, name), _ram_manager(env, name),
		_initial_thread(_cpu, _pd.cap(), name),
		_address_space(_pd.address_space()),
		_log_service("LOG"), _timer_service("Timer"),
		_child(_rom.dataspace(), Dataspace_capability(),
				_pd, _pd, _ram_manager.ram, _ram_manager.ram, _cpu, _initial_thread,
				env.rm(), _address_space, _child_ep.rpc_ep(), *this)
	{ }

	const char *name() const { return _name; }

	Service *resolve_session_request(const char *service_name, const char *args)
	{
		if(!strcmp(service_name, "LOG")) return &_log_service;
		if(!strcmp(service_name, "Timer")) return &_timer_service;
		return 0;
	}
};

size_t Component::stack_size() { return 64*1024; }

void Component::construct(Genode::Env &env)
{
	const char name[] = "sheep_counter";
	Heap md_alloc { env.ram(), env.rm() };

	My_child child0 { env, name };

	sleep_forever();
}
