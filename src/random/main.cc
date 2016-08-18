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
private:
	Ram_connection _parent_ram;
public:
	My_ram_session_component(Env &env, const char *name) : _parent_ram(env, name)
	{ }

	~My_ram_session_component()
	{ }

	Ram_session_capability parent_cap()
	{
		return _parent_ram;
	}

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
	String<32>  _name;
	Env        &_env;
	Entrypoint  _child_ep;
	Entrypoint  _extra_ep;
	struct Resources
	{
		Pd_connection            pd;
		Cpu_connection           cpu;
		Rom_connection           rom;
		My_ram_session_component ram;
		Resources(Env &env, Entrypoint &ep, const char *name)
		:
			pd(env, name), cpu(env, name), rom(env, name), ram(env, name)
		{
			ep.manage(ram);
			ram.ref_account(env.ram_session_cap());
			env.ram().transfer_quota(ram.parent_cap(), 1024*1024);
		}
	} _resources;
	Child::Initial_thread _initial_thread;
	Region_map_client     _address_space;
	Parent_service        _log_service;
	Parent_service        _timer_service;
	Child                 _child;
public:
	My_child(Env &env, const char *name)
	:
		_name           (name),
		_env            (env),
		_child_ep       (env, 64*1024, "child_ep"),
		_extra_ep       (env, 64*1024, "extra_ep"),
		_resources      (env, _child_ep, _name.string()),
		_initial_thread (_resources.cpu, _resources.pd.cap(), _name.string()),
		_address_space  (_resources.pd.address_space()),
		_log_service    ("LOG"),
		_timer_service  ("Timer"),
		_child(_resources.rom.dataspace(), Dataspace_capability(),
				_resources.pd,        _resources.pd,
				_resources.ram.cap(), _resources.ram,
				_resources.cpu,       _initial_thread,
				env.rm(), _address_space, _extra_ep.rpc_ep(), *this)
	{ }

	const char *name() const { return _name.string(); }

	Service *resolve_session_request(const char *service_name, const char *args)
	{
		if(!strcmp(service_name, "LOG"))   return &_log_service;
		if(!strcmp(service_name, "Timer")) return &_timer_service;
		return 0;
	}
};

size_t Component::stack_size() { return 64*1024; }

void Component::construct(Genode::Env &env)
{
	My_child child0 { env, "sheep_counter" };

	sleep_forever();
}
