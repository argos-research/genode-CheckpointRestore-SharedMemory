/*
 * \brief  Template test
 * \author Denis Huber
 * \date   2016-08-20
 */

/* Genode includes */
#include <base/env.h>
#include <base/log.h>
#include <base/component.h>
#include <base/child.h>
#include <base/sleep.h>
#include <ram_session/connection.h>
#include <rom_session/connection.h>
#include <cpu_session/connection.h>
#include <cap_session/connection.h>
#include <pd_session/connection.h>
#include <region_map/client.h>

using namespace Genode;

class My_child : public Genode::Child_policy
{
private:

	struct Resources
	{
		Genode::Pd_connection  pd;
		Genode::Ram_connection ram;
		Genode::Cpu_connection cpu;

		Resources(char const *label)
		: pd(label)
		{
			using namespace Genode;

			/* transfer some of our own ram quota to the new child */
			enum { CHILD_QUOTA = 1*1024*1024 };
			ram.ref_account(env()->ram_session_cap());
			env()->ram_session()->transfer_quota(ram.cap(), CHILD_QUOTA);

			/* register handler for unresolvable page faults */
			Region_map_client address_space(pd.address_space());
		}
	} _resources;

	Genode::Child::Initial_thread _initial_thread;

	/*
	 * The order of the following members is important. The services must
	 * appear before the child to ensure the correct order of destruction.
	 * I.e., the services must remain alive until the child has stopped
	 * executing. Otherwise, the child may hand out already destructed
	 * local services when dispatching an incoming session call.
	 */
	Genode::Rom_connection    _elf;
	Genode::Parent_service    _log_service;
	Genode::Parent_service    _rm_service;
	Genode::Parent_service    _timer_service;
	Genode::Region_map_client _address_space { _resources.pd.address_space() };
	Genode::Child             _child;

public:

	/**
	 * Constructor
	 */
	My_child(Genode::Entrypoint &ep, char const *label)
	:
		_resources(label),
		_initial_thread(_resources.cpu, _resources.pd, label),
		_elf(label),
		_log_service("LOG"), _rm_service("RM"), _timer_service("Timer"),
		_child(_elf.dataspace(), Genode::Dataspace_capability(),
		       _resources.pd,  _resources.pd,
		       _resources.ram, _resources.ram,
		       _resources.cpu, _initial_thread,
		       *Genode::env()->rm_session(), _address_space, ep.rpc_ep(), *this)
	{ }


	/****************************
	 ** Child-policy interface **
	 ****************************/

	const char *name() const { return "child"; }

	Genode::Service *resolve_session_request(const char *service, const char *)
	{
		/* forward white-listed session requests to our parent */
		return !Genode::strcmp(service, "LOG")   ? &_log_service
		     : !Genode::strcmp(service, "RM")    ? &_rm_service
		     : !Genode::strcmp(service, "Timer") ? &_timer_service
		     : 0;
	}

	void filter_session_args(const char *service,
	                         char *args, Genode::size_t args_len)
	{
		/* define session label for sessions forwarded to our parent */
		Genode::Arg_string::set_arg_string(args, args_len, "label", "child");
	}
};

size_t Component::stack_size() { return 64*1024; }

void Component::construct(Genode::Env &env)
{
	log("Hello world!");

	Genode::Entrypoint ep(env, 16*1024, "child_ep");

    My_child child0(ep, "sheep_counter");

    Genode::sleep_forever();
}
