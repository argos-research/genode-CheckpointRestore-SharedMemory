#include <base/component.h>
#include <timer_session/connection.h>
#include <region_map/client.h>

#include <base/printf.h>
#include <base/log.h>
#include <base/env.h>
#include <base/sleep.h>
#include <base/child.h>
#include <pd_session/connection.h>
#include <rm_session/connection.h>
#include <ram_session/connection.h>
#include <rom_session/connection.h>
#include <cpu_session/connection.h>
#include <cap_session/connection.h>
#include <rm_session/client.h>


enum {
	MANAGED_SIZE = 0x00010000,
	PAGE_SIZE    = 4096,
};


using namespace Genode;


/**
 * Region-manager fault handler resolves faults by attaching new dataspaces
 */
class Local_fault_handler : public Thread_deprecated<4096>
{
	private:

		Region_map      &_region_map;
		Signal_receiver &_receiver;

	public:

		Local_fault_handler(Region_map &region_map, Signal_receiver &receiver)
		:
			Thread_deprecated("local_fault_handler"),
			_region_map(region_map), _receiver(receiver)
		{ }

		void handle_fault()
		{
			Region_map::State state = _region_map.state();

			printf("region-map state is %s, pf_addr=0x%lx\n",
			       state.type == Region_map::State::READ_FAULT  ? "READ_FAULT"  :
			       state.type == Region_map::State::WRITE_FAULT ? "WRITE_FAULT" :
			       state.type == Region_map::State::EXEC_FAULT  ? "EXEC_FAULT"  : "READY",
			       state.addr);

			printf("allocate dataspace and attach it to sub region map\n");
			Dataspace_capability ds = env()->ram_session()->alloc(PAGE_SIZE);
			_region_map.attach_at(ds, state.addr & ~(PAGE_SIZE - 1));

			printf("returning from handle_fault\n");
		}

		void entry()
		{
			while (true) {
				printf("fault handler: waiting for fault signal\n");
				Signal signal = _receiver.wait_for_signal();
				printf("received %u fault signals\n", signal.num());
				for (unsigned i = 0; i < signal.num(); i++)
					handle_fault();
			}
		}
};




class Test_child : public Child_policy
{
	private:

		enum { STACK_SIZE = 8*1024 };

		/*
		 * Entry point used for serving the parent interface
		 */
		Rpc_entrypoint _entrypoint;

		Region_map_client     _address_space;
		Pd_session_client     _pd;
		Ram_session_client    _ram;
		Cpu_session_client    _cpu;
		Child::Initial_thread _initial_thread;

		Child _child;

		Parent_service _log_service;
		Parent_service _timer_service;

	public:

		/**
		 * Constructor
		 */
		Test_child(Genode::Dataspace_capability    elf_ds,
		           Genode::Pd_connection          &pd,
		           Genode::Ram_session_capability  ram,
		           Genode::Cpu_session_capability  cpu,
		           Genode::Cap_session            *cap)
		:
			_entrypoint(cap, STACK_SIZE, "child", false),
			_address_space(pd.address_space()), _pd(pd), _ram(ram), _cpu(cpu),
			_initial_thread(_cpu, _pd, "child"),
			_child(elf_ds, Dataspace_capability(), _pd, _pd, _ram, _ram,
			       _cpu, _initial_thread, *env()->rm_session(), _address_space,
			       _entrypoint, *this),
			_log_service("LOG"),
			_timer_service("Timer")
		{
			/* start execution of the new child */
			_entrypoint.activate();
		}


		/****************************
		 ** Child-policy interface **
		 ****************************/

		const char *name() const { return "rmchild"; }

		Service *resolve_session_request(const char *service, const char *)
		{
			/* forward white-listed session requests to our parent */
            if(strcmp(service, "LOG") == 0)
                return &_log_service;
            else if(strcmp(service, "Timer") == 0)
                return &_timer_service;
			else
                return 0;
		}

		void filter_session_args(const char *service,
		                         char *args, size_t args_len)
		{
			/* define session label for sessions forwarded to our parent */
			Arg_string::set_arg_string(args, args_len, "label", "child");
		}
};


void Component::construct(Genode::Env &env)
{
    log("Hello from rmcr");
	Timer::Connection timer(env);
    static Rom_connection rom("sheep_counter");
    Dataspace_capability elf_ds = rom.dataspace();

    /* create environment for new child */
	static Pd_connection  pd;
	static Ram_connection ram;
	static Cpu_connection cpu;
	static Cap_connection cap;

	/* transfer some of our own ram quota to the new child */
	enum { CHILD_QUOTA = 1*1024*1024 };
	ram.ref_account(Genode::env()->ram_session_cap());
	Genode::env()->ram_session()->transfer_quota(ram.cap(), CHILD_QUOTA);

	static Signal_receiver fault_handler;

	/* register fault handler at the child's address space */
	static Signal_context signal_context;
	Region_map_client address_space(pd.address_space());
	address_space.fault_handler(fault_handler.manage(&signal_context));

	/* create child */
	static Test_child child(elf_ds, pd, ram.cap(), cpu.cap(), &cap);
    log("child created");

	while(true)
    {
		log("wait for region-manager fault");
		Signal s = fault_handler.wait_for_signal();
		log("received ", s.num(), " pagefaults");
		for(unsigned int i=0;i<s.num();i++)
		{
			log("received region-manager fault signal, request fault state");

			Region_map::State state = address_space.state();

			char const *state_name =
				state.type == Region_map::State::READ_FAULT  ? "READ_FAULT"  :
				state.type == Region_map::State::WRITE_FAULT ? "WRITE_FAULT" :
				state.type == Region_map::State::EXEC_FAULT  ? "EXEC_FAULT"  : "READY";

			log("rm session state is ", state_name, ", pf_addr=", Hex(state.addr));

			/* ignore spuriuous fault signal */
			if (state.type == Region_map::State::READY) {
				log("ignoring spurious fault signal");
				continue;
			}

			address_space.processed(state);
		}
    }


}
