/*
 * \brief  Using fault handler
 * \author Denis Huber
 * \date   2016-08-30
 */

/* Genode includes */
#include <base/env.h>
#include <base/log.h>
#include <base/component.h>

using namespace Genode;

struct Main
{
	Env                  &env;
	Entrypoint            pager_ep;
	Signal_handler<Main>  sigh;

	void _handle_fault()
	{
		Region_map::State state = env.rm().state();

		log("  Region map state is ",
				state.type == Region_map::State::READ_FAULT  ? "READ_FAULT"  :
				state.type == Region_map::State::WRITE_FAULT ? "WRITE_FAULT" :
				state.type == Region_map::State::EXEC_FAULT  ? "EXEC_FAULT"  : "READY",
				" pf_addr=", Hex(state.addr));

		if(state.type == Region_map::State::READY)
			return;

		log("  Allocating dataspace and attaching it to the region map");
		// Creating dataspace
		Dataspace_capability ds = env.ram().alloc(4096);
		// Attaching dataspace to the pagefault address and page-aligned
		env.rm().attach_at(ds, state.addr & ~(4096 - 1));
	}

	Main(Env &env)
	:
		env(env),
		pager_ep(env, 16*1024, "pager_ep"),
		sigh(pager_ep, *this, &Main::_handle_fault)
	{
		log("--- pf-signal_handler started ---");

		// Assigning pagefault handler to address space
		env.rm().fault_handler(sigh);

		// Causing page fault
		*((unsigned int*)0x1000) = 1;

		log("--- pf-signal_handler ended ---");
	}
};

namespace Component
{
	size_t stack_size ()                 { return 16*1024;        }
	void   construct  (Genode::Env &env) { static Main main(env); }
}
