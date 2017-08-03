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

#include "../../rtcr/multicore/validator_session/connection.h"
#include "../../rtcr/util/sizes.h"

namespace Fiasco {
#include <l4/sys/kdebug.h>
}

Genode::size_t Component::stack_size() { return 16*1024; }

void Component::construct(Genode::Env &env)
{
	//enter_kdebug("before restore");
	using namespace Genode;

	log("\033[32mAffinity space of child counter env: ",env.cpu().affinity_space().width(), " X ", env.cpu().affinity_space().height()," total:",env.cpu().affinity_space().total(),"\033[0m");


	log("Creating Timer session.");
	Timer::Connection timer(env);


	log("Allocating and attaching memory and its dataspace.");
	Dataspace_capability ds_cap = env.ram().alloc(Rtcr::DS_ALLOC_SIZE);

	char *addr = env.rm().attach(ds_cap);

	addr[0] = 'H';
	addr[1] = 'A';
	addr[2] = 'L';
	addr[3] = 'L';
	addr[4] = 'O';

	log("Dataspace attached at \033[31m", &addr,"\033[0m");
	log("First Char: \033[31m", (char)addr[0],"\033[0m");
	log("Second Char: \033[31m", (char)addr[1],"\033[0m");
	log("Third Char: \033[31m", (char)addr[2],"\033[0m");
	log("Fourth Char: \033[31m", (char)addr[3],"\033[0m");
	log("Fifth Char: \033[31m", (char)addr[4],"\033[0m");


	log("Allocating and attaching memory and its dataspace.");
	Dataspace_capability ds_cap2 = env.ram().alloc(Rtcr::DS_ALLOC_SIZE);

	char *addr2 = env.rm().attach(ds_cap2);

	addr2[0] = 'H';
	addr2[1] = 'A';
	addr2[2] = 'L';
	addr2[3] = 'L';
	addr2[4] = 'O';

	log("Dataspace attached at \033[31m", &addr2,"\033[0m");
	log("First Char: \033[31m", (char)addr2[0],"\033[0m");
	log("Second Char: \033[31m", (char)addr2[1],"\033[0m");
	log("Third Char: \033[31m", (char)addr2[2],"\033[0m");
	log("Fourth Char: \033[31m", (char)addr2[3],"\033[0m");
	log("Fifth Char: \033[31m", (char)addr2[4],"\033[0m");



	log("Allocating and attaching memory and its dataspace.");
		Dataspace_capability ds_cap5 = env.ram().alloc(Rtcr::DS_ALLOC_SIZE*4);

		char *addr5 = env.rm().attach(ds_cap5);

		addr5[0] = 'H';
		addr5[1] = 'A';
		addr5[2] = 'L';
		addr5[3] = 'L';
		addr5[4] = 'O';

		log("Dataspace attached at \033[31m", &addr5,"\033[0m");
		log("First Char: \033[31m", (char)addr5[0],"\033[0m");
		log("Second Char: \033[31m", (char)addr5[1],"\033[0m");
		log("Third Char: \033[31m", (char)addr5[2],"\033[0m");
		log("Fourth Char: \033[31m", (char)addr5[3],"\033[0m");
		log("Fifth Char: \033[31m", (char)addr5[4],"\033[0m");



	log("Allocating and attaching memory and its dataspace.");
	Dataspace_capability ds_cap3 = env.ram().alloc(Rtcr::DS_ALLOC_SIZE);

	char *addr3 = env.rm().attach(ds_cap3);

	addr3[0] = 'H';
	addr3[1] = 'A';
	addr3[2] = 'L';
	addr3[3] = 'L';
	addr3[4] = 'O';

	log("Dataspace attached at \033[31m", &addr3,"\033[0m");
	log("First Char: \033[31m", (char)addr3[0],"\033[0m");
	log("Second Char: \033[31m", (char)addr3[1],"\033[0m");
	log("Third Char: \033[31m", (char)addr3[2],"\033[0m");
	log("Fourth Char: \033[31m", (char)addr3[3],"\033[0m");
	log("Fifth Char: \033[31m", (char)addr3[4],"\033[0m");



	log("Allocating and attaching memory and its dataspace.");
		Dataspace_capability ds_cap4 = env.ram().alloc(Rtcr::DS_ALLOC_SIZE);

		char *addr4 = env.rm().attach(ds_cap4);

		addr4[0] = 'H';
		addr4[1] = 'A';
		addr4[2] = 'L';
		addr4[3] = 'L';
		addr4[4] = 'O';

		log("Dataspace attached at \033[31m", &addr4,"\033[0m");
		log("First Char: \033[31m", (char)addr4[0],"\033[0m");
		log("Second Char: \033[31m", (char)addr4[1],"\033[0m");
		log("Third Char: \033[31m", (char)addr4[2],"\033[0m");
		log("Fourth Char: \033[31m", (char)addr4[3],"\033[0m");
		log("Fifth Char: \033[31m", (char)addr4[4],"\033[0m");
	//env.parent().upgrade(timer, "ram_quota=8K");
	//env.parent().upgrade(env.ram_session_cap(), "ram_quota=24K");
//	env.rm().detach(*addr);




	//env.parent().upgrade(timer, "ram_quota=8K");

}
