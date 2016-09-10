/*
 * \brief  Capability numbering after deleting and freeing a capability and creating new capability
 * \author Denis Huber
 * \date   2016-08-20
 */

/* Genode includes */
#include <base/env.h>
#include <base/log.h>
#include <base/component.h>

using namespace Genode;

size_t Component::stack_size() { return 64*1024; }

void Component::construct(Genode::Env &env)
{
	log("--- Reusing cap started ---");

	Dataspace_capability ds0 = env.ram().alloc(4096);
	Dataspace_capability ds1 = env.ram().alloc(4096);
	Dataspace_capability ds2 = env.ram().alloc(4096);

	unsigned int* addr0 = env.rm().attach(ds0);
	unsigned int* addr1 = env.rm().attach(ds1);
	unsigned int* addr2 = env.rm().attach(ds2);

	log("ds0 = ", ds0.local_name(), ", addr0 = ", addr0);
	log("ds1 = ", ds1.local_name(), ", addr1 = ", addr1);
	log("ds2 = ", ds2.local_name(), ", addr2 = ", addr2);

	log("Writing to ds0");

	*addr0 = 13;
	*(addr0+1) = 42;

	log("Reading ds0: ", *addr0, ", ", *(addr0+1));

	log("Freeing ds0 and creating ds3");

	// Implicit dataspace detachment
	env.ram().free(static_cap_cast<Ram_dataspace>(ds0));

	// Creating new capability reuses the freed capability number
	Dataspace_capability ds3   = env.ram().alloc(2*4096);
	unsigned int*        addr3 = env.rm().attach(ds3);
	log("ds3 = ", ds3.local_name(), ", addr3 = ", addr3);
	log("Read ds3: ", *addr3, ", ", *(addr3+1));

	log("--- Reusing cap ended ---");
}
