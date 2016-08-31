/*
 * \brief  Using fault handler
 * \author Denis Huber
 * \date   2016-08-30
 */

/* Genode includes */
#include <base/env.h>
#include <base/log.h>
#include <base/component.h>
#include <base/sleep.h>

using namespace Genode;

struct Faulting_thread : Thread
{
	Faulting_thread(Env &env)
	:
		Thread(env, "faulting thread", 8*1024)
	{ }

	void entry()
	{
		*((unsigned int*)0x1000) = 1;

		log("  Page fault handled.");
		log("--- pf-signal_handler ended ---");
	}
};

class Main
{
private:
	Env                  &_env;
	Signal_handler<Main>  _sigh          {_env.ep(), *this, &Main::_handle_fault};
	Faulting_thread       _fault_thread  {_env};


	void _handle_fault()
	{
		Region_map::State state = _env.rm().state();

		log("  Region map state is ",
				state.type == Region_map::State::READ_FAULT  ? "READ_FAULT"  :
				state.type == Region_map::State::WRITE_FAULT ? "WRITE_FAULT" :
				state.type == Region_map::State::EXEC_FAULT  ? "EXEC_FAULT"  : "READY",
				" pf_addr=", Hex(state.addr));

		if(state.type == Region_map::State::READY)
			return;

		log("  Allocating dataspace and attaching it to the region map");
		// Creating dataspace
		Dataspace_capability ds = _env.ram().alloc(4096);
		// Attaching dataspace to the pagefault address (page-aligned)
		_env.rm().attach_at(ds, state.addr & ~(4096 - 1));
	}

public:
	Main(Env &env)
	:
		_env(env)
	{
		log("--- pf-signal_handler started ---");

		// Assigning page fault handler to address space
		_env.rm().fault_handler(_sigh);

		// Starting thread which causes the page fault
		_fault_thread.start();

	}
};

namespace Component
{
	size_t stack_size ()                 { return 16*1024;        }
	void   construct  (Genode::Env &env) { static Main main(env); }
}
