/*
 * \brief  Using old fault handler objects (e.g. Signal_receiver)
 * \author Denis Huber
 * \date   2016-08-30
 */

/* Genode includes */
#include <base/env.h>
#include <base/log.h>
#include <base/component.h>
#include <region_map/client.h>

using namespace Genode;

class Local_fault_handler : public Thread
{
	Env             &_env;
	Region_map      &_region_map;
	Signal_receiver &_receiver;

public:
	Local_fault_handler(Env &env, Region_map &region_map, Signal_receiver &receiver)
	:
		Thread(env, "fault_handler", 0x1000),
		_env(env), _region_map(region_map), _receiver(receiver)
	{ }

	void handle_fault()
	{
		Region_map::State state = _region_map.state();

		log("    Region map state is ",
				state.type == Region_map::State::READ_FAULT  ? "READ_FAULT"  :
				state.type == Region_map::State::WRITE_FAULT ? "WRITE_FAULT" :
				state.type == Region_map::State::EXEC_FAULT  ? "EXEC_FAULT"  : "READY",
				" pf_addr=", Hex(state.addr));

		log("    Allocating dataspace and attaching it to the region map");

		// Create a new dataspace (page-size)
		Dataspace_capability ds = _env.ram().alloc(4096);

		// Attach dataspace to region map at the fault address and page-aligned
		//   4096  = 0x00001000
		//   4095  = 0x00000fff
		// ~(4095) = 0xfffff000
		// 0x12345678 & 0xfffff000 = 0x12345000
		// state.addr & ~(4095) omits the last 12 bits of the address
		_region_map.attach_at(ds, state.addr & ~(4096 - 1));
	}

	void entry()
	{
		while(true)
		{
			log("  Fault handler: waiting for fault signal");

			// Wait for signals to arrive (blocking)
			Signal signal = _receiver.wait_for_signal();

			log("    Received ", signal.num(), " fault signals");

			// Handle all signals
			for(unsigned int i = 0; i < signal.num(); ++i)
				handle_fault();
		}
	}
};

size_t Component::stack_size() { return 64*1024; }

void Component::construct(Genode::Env &env)
{

	log("--- pf-receiver started ---");

	Region_map      &address_space = env.rm();
	Signal_receiver  receiver;
	Signal_context   signal_context;

	// Assigning receiver as receiver of page faults
	address_space.fault_handler(receiver.manage(&signal_context));

	// Start fault handler thread
	Local_fault_handler fault_handler(env, address_space, receiver);
	fault_handler.start();

	// Create page fault (WRITE)
	*((unsigned int*)0x1000) = 1;

	log("--- pf-receiver ended ---");

}
