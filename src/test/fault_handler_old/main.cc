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
#include <rm_session/connection.h>

using namespace Genode;

class Local_fault_handler : public Thread
{
	Env             &_env;
	Region_map      &_region_map0;
	Region_map      &_region_map1;
	Signal_receiver &_receiver;

public:
	Local_fault_handler(Env &env, Region_map &region_map0, Region_map &region_map1, Signal_receiver &receiver)
	:
		Thread(env, "fault_handler", 0x1000),
		_env(env), _region_map0(region_map0), _region_map1(region_map1), _receiver(receiver)
	{ }

	void handle_fault()
	{
		Region_map::State state0 = _region_map0.state();
		Region_map::State state1 = _region_map1.state();
		log("    region_map0 state is ",
			state0.type == Region_map::State::READ_FAULT  ? "READ_FAULT"  :
			state0.type == Region_map::State::WRITE_FAULT ? "WRITE_FAULT" :
			state0.type == Region_map::State::EXEC_FAULT  ? "EXEC_FAULT"  : "READY",
			" pf_addr=", Hex(state0.addr));

		log("    region_map1 state is ",
			state1.type == Region_map::State::READ_FAULT  ? "READ_FAULT"  :
			state1.type == Region_map::State::WRITE_FAULT ? "WRITE_FAULT" :
			state1.type == Region_map::State::EXEC_FAULT  ? "EXEC_FAULT"  : "READY",
			" pf_addr=", Hex(state1.addr));

		if(state0.type != Region_map::State::READY)
		{
			log("    Allocating dataspace");

			// Create a new dataspace (page-size)
			Dataspace_capability ds = _env.ram().alloc(4096);

			log("    Attaching dataspace to region_map0");
			// Attach dataspace to region map at the fault address and page-aligned
			//   4096  = 0x00001000
			//   4095  = 0x00000fff
			// ~(4095) = 0xfffff000
			// 0x12345678 & 0xfffff000 = 0x12345000
			// state.addr & ~(4095) omits the last 12 bits of the address
			_region_map0.attach_at(ds, state0.addr & ~(4096 - 1));
		}
		else if(state1.type != Region_map::State::READY)
		{
			log("    Allocating dataspace");

			// Create a new dataspace (page-size)
			Dataspace_capability ds = _env.ram().alloc(4096);

			log("    Attaching dataspace to the region_map1");
			// Attach dataspace to region map at the fault address and page-aligned
			//   4096  = 0x00001000
			//   4095  = 0x00000fff
			// ~(4095) = 0xfffff000
			// 0x12345678 & 0xfffff000 = 0x12345000
			// state.addr & ~(4095) omits the last 12 bits of the address
			_region_map1.attach_at(ds, state1.addr & ~(4096 - 1));
		}
		else
		{
			warning("No faulting state found!");
		}
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

	Rm_connection rm_service {env};

	Region_map &address_space = env.rm();

	Capability<Region_map> rm_cap0 = rm_service.create(2*4096);
	Capability<Region_map> rm_cap1 = rm_service.create(2*4096);

	log("Created 2 Region_maps: ", rm_cap0.local_name(), " and ", rm_cap1.local_name());

	Region_map_client rm_client0 {rm_cap0};
	Region_map_client rm_client1 {rm_cap1};

	Signal_receiver  receiver;
	Signal_context   signal_context0;
	Signal_context   signal_context1;

	// Assigning receiver as receiver of page faults
	rm_client0.fault_handler(receiver.manage(&signal_context0));
	rm_client1.fault_handler(receiver.manage(&signal_context1));

	// Start fault handler thread
	Local_fault_handler fault_handler(env, rm_client0, rm_client1, receiver);
	fault_handler.start();

	unsigned int* addr0 = address_space.attach_at(rm_client0.dataspace(), 0x8000);
	unsigned int* addr1 = address_space.attach_at(rm_client0.dataspace(), 0x8000*2);

	log("Attached dataspaces to ", addr0, " and ", addr1);

	// Create page fault (WRITE)
	//*addr0 = 42;
	*(addr1+1) = 42;

	log("--- pf-receiver ended ---");

}
