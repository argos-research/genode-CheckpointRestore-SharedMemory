/*
 * \brief  Testing managed dataspaces
 * \author Denis Huber
 * \date   2016-09-01
 */

/* Genode includes */
#include <base/env.h>
#include <base/log.h>
#include <base/component.h>
#include <rm_session/connection.h>
#include <region_map/client.h>

using namespace Genode;

size_t Component::stack_size() { return 64*1024; }

void Component::construct(Genode::Env &env)
{
	log("--- Managed dataspace test started ---");
	unsigned char* addr = 0;

	Dataspace_capability ds0 = env.ram().alloc(4096);
	Dataspace_capability ds1 = env.ram().alloc(4096);

	Rm_connection          rm_service        {env};
	// Extra Region_map has a limited size and shall fit into the address space
	Capability<Region_map> my_region_map     {rm_service.create(3*4096)};
	Region_map_client      region_map_client {my_region_map};

	log("Attaching 2 ds in new region map");
	// The dataspace is attached at local_addr 0
	addr = region_map_client.attach(ds0);
	log("  Attached on address ", addr);
	// This dataspace is attached at local_addr 0x2000
	addr = region_map_client.attach_at(ds1, 0x2000);
	log("  Attached on address ", addr);

	log("Attaching new region map as a dataspace to env region map");
	addr = env.rm().attach(region_map_client.dataspace());
	log("  Attached on address ", addr);
	log("  Physically backed up positions are: ",
			"[", addr,        ", ", addr+0x1000, ") and ",
			"[", addr+0x2000, ", ", addr+0x3000, ")");
	log("Note, that the positions are different from local addresses inside the new region map!");

	// When a page fault happens inside a new region map, the addresses are transformed into local addresses of the new region map!
	// TODO create test for that



	log("--- Managed dataspace test ended ---");
}
