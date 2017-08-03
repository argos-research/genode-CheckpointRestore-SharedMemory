/*
 * \brief  Brief test case for testing affinity subspacing for runtime child creation
 * \author Stephan Lex
 * \date   2017-08-02
 */

/* Genode include */
#include <base/component.h>
#include <base/signal.h>
#include <base/sleep.h>
#include <base/log.h>
#include <base/child.h>
#include <timer_session/connection.h>

#include <pd_session/connection.h>
#include <cpu_session/connection.h>
#include <ram_session/connection.h>
#include <rom_session/connection.h>



struct Main
{
	Genode::Env              &env;
	Genode::Heap              heap            { env.ram(), env.rm() };
	Genode::Service_registry  parent_services { };

	/*
	 * Test class for implementing the Child_policy interface
	 */
	class Test_child : public Genode::Child_policy {
	private:
		Genode::Service_registry _parent_services;
		Genode::Allocator &_alloc;

	public:
		Test_child(Genode::Allocator &alloc, Genode::Service_registry parent_services) :
			_alloc(alloc),
			_parent_services(parent_services)
		{}

		const char *name() const { return "Test_Child"; }

		Genode::Service *resolve_session_request(const char *service_name, const char *args)
		{
			Genode::Service *service = 0;

			// Service known from parent?
			service = _parent_services.find(service_name);
			if(service)
				return service;

			// Service not known, cannot intercept it
			if(!service)
			{
				service = new (_alloc) Genode::Parent_service(service_name);
				_parent_services.insert(service);
				Genode::warning("Unknown service: ", service_name);
			}

			return service;
		}

		void filter_session_args(const char *service, char *args, Genode::size_t args_len)
		{
			Genode::Arg_string::set_arg_string(args, args_len, "label", "");
		}
	};


	Main(Genode::Env &env_) : env(env_)
	{
		using namespace Genode;

		Timer::Connection timer { env };

		log("parent started ...");
		log("parent cpu cap: ", env.cpu_session_cap());
		log("parent ram cap: ", env.ram_session_cap());

		// Declaring both children
		Child* counter;
		Child* validator;

		/*
		** The idea here is after assigning the three rightmost cores to the parent in the run script
		** to define two affinities with a subspace of 1, so every child should run on one dedicated core
		** while the parent takes the other ?
		**
		** CORES:	O		O		O		O
		** 			^		^		^		^
		** 		(	|		|		|		|	) Init subspace by run script
		** 			|	(	|		|		|	) Parent subspace by run script
		** 			|		|	(	|	)(	|	) Subspaces of children defined in code
		**
		** 		  INIT	 PARENT COUNTER  VALIDATOR
		*/

		const Affinity &aff_count{Genode::Affinity::Space(1), Genode::Affinity::Location(1, 0)};
		const Affinity &aff_val{Genode::Affinity::Space(1), Genode::Affinity::Location(2, 0)};




		// Create entrypoint and policy for the children
		Entrypoint child_ep(env, 16*1024, "child_ep");
		Test_child child_policy(heap, parent_services);

		// Create rom, pd, cpu and ram sessions for child counter
		// Passing the cpu session the first affinity
		Rom_connection rom_c(env, "child_counter");
		Pd_connection pd_c(env, "counter_pd");
		Cpu_connection cpu_c(env, "counter_cpu", 0, aff_count);
		Ram_connection ram_c(env, "counter_ram");

		ram_c.ref_account(env.ram_session_cap());
		log(env.ram().transfer_quota(ram_c.cap(), 1024*1024));


		// Create the initial thread and pd-local region map for the child
		Child::Initial_thread child_counter_it(cpu_c, pd_c.cap(), "child_counter_it");
		Region_map_client rm_c(pd_c.address_space());

		// Create the actual child
		counter = new (heap) Child(
				rom_c.dataspace(),
				Dataspace_capability(),
				pd_c.cap(), pd_c,
//				env.ram_session_cap(), env.ram(),
				ram_c.cap(), ram_c,
				cpu_c.cap(), child_counter_it,
				env.rm(), rm_c, child_ep.rpc_ep(), child_policy);

		log("child counter started ...");
		log("counter parent cap: ",counter->parent_cap());
		log("counter pd cap: ",counter->pd_session_cap());
		log("counter cpu cap: ",counter->cpu_session_cap());
		log("counter ram cap: ",counter->ram_session_cap());
		log("counter heap: ",counter->heap());
		log("counter main thread cap: ",counter->main_thread_cap());
		timer.msleep(5000);



		// Create rom, pd, cpu and ram sessions for child validator
		// Passing the cpu session the second affinity
		Rom_connection rom_v(env, "child_validator");
		Pd_connection pd_v(env, "validator_pd");
		Cpu_connection cpu_v(env, "validator_cpu", 0, aff_val);
		Ram_connection ram_v(env, "validator_ram");

		ram_v.ref_account(env.ram_session_cap());
		log(env.ram().transfer_quota(ram_v.cap(), 1024*1024));


		// Create the initial thread and pd-local region map for the child validator
		Child::Initial_thread child_validator_it(cpu_v, pd_v.cap(), "child_validator_it");
		Region_map_client rm_v(pd_v.address_space());

		// Create the actual child
		validator = new (heap) Child (
				rom_v.dataspace(),
				Dataspace_capability(),
				pd_v.cap(), pd_v,
				ram_v.cap(), ram_v,
				cpu_v.cap(), child_validator_it,
				env.rm(), rm_v, child_ep.rpc_ep(), child_policy);

		log("child validator started ...");
		log("validator parent cap: ",validator->parent_cap());
		log("validator pd cap: ",validator->pd_session_cap());
		log("validator cpu cap: ",validator->cpu_session_cap());
		log("validator ram cap: ",validator->ram_session_cap());
		log("validator heap: ",validator->heap());
		log("validator main thread cap: ",validator->main_thread_cap());

		Genode::sleep_forever();
	}
};

Genode::size_t Component::stack_size() { return 32*1024; }

void Component::construct(Genode::Env &env)
{
	static Main main(env);
}
