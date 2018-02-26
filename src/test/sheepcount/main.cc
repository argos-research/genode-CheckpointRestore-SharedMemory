/*
 * \brief  Testprogram which just counts sheeps
 * \author Denis Huber
 * \date   2016-08-04
 *
 * This program is a target for the rtcr service. It counts a sheep,
 * prints the number of the sheep and goes to sleep between each 
 * iteration. This component will be checkpointed serialized, 
 * (transfered), deserialized, and restored. It does not know that it is
 * being checkpointed.
 */

#include <base/component.h>
#include <timer_session/connection.h>
#include <base/log.h>
#include <rm_session/connection.h>
#include <region_map/client.h>

//namespace Fiasco {
//#include <l4/sys/kdebug.h>
//}
enum { MANAGED_ADDR = 0x10000000 };

Genode::size_t Component::stack_size() { return 16*1024; }

void Component::construct(Genode::Env &env)
{
	//enter_kdebug("before restore");
	using namespace Genode;
	log("Creating Timer session.");
	Timer::Connection timer(env);


	timer.msleep(3000);
	log("Allocating and attaching memory and its dataspace.");
	Dataspace_capability ds_cap = env.ram().alloc(4096);
	unsigned* addr = env.rm().attach(ds_cap);
//    unsigned int counter = 0;
//	unsigned int *addr = &counter;
//	unsigned int *addr = (unsigned int*) MANAGED_ADDR;
	//addr[0x432]
	addr_t base_addr = 0x5b;
	unsigned &n = addr[base_addr];
	unsigned test =50;
	n = 0xfffffff0; //144

	log("base value: ", (unsigned int) n, ", base addr: ", Genode::Hex(base_addr));

	//env.parent().upgrade(timer, "ram_quota=8K");
	//env.parent().upgrade(env.ram_session_cap(), "ram_quota=24K");

	while(1)
	{
        log(n, " sheep. zzZ", test);
        n=test;
		//n++;
        test++;
		timer.msleep(1000);
	}

}
