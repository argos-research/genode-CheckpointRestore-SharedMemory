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

#include "../../rtcr/multicore/validator_session/connection.h"


namespace Fiasco {
#include <l4/sys/kdebug.h>
}


Genode::size_t Component::stack_size() { return 16*1024; }

void Component::construct(Genode::Env &env)
{
	//enter_kdebug("before restore");
	using namespace Genode;

	log("Affinity space of validator env: \033[32m",env.cpu().affinity_space().width(), " X ", env.cpu().affinity_space().height(),"\033[0m");

	log("Creating Timer session.");
	Timer::Connection timer(env);

	log("Creating Validator session.");
	Rtcr::Validator_connection validator(env);

	while(validator.dataspace_available())
	{
		log("--------------------------------------------------------------------------");
		log("Attaching memory and its dataspace.");
		Dataspace_capability ds_cap = validator.get_dataspace();
		log("Dataspace capability: ",ds_cap);
		char *addr = env.rm().attach(ds_cap);

		if(addr[0] == 'H' && addr[1] == 'A' && addr[2] == 'L' && addr[3] == 'L' && addr[4] == 'O' )
		{
			log("Memory successfully checkpointed");
		}
		else
		{
			log("First Char: ",addr[0]);
			log("Second Char: ",addr[1]);
			log("Third Char: ",addr[2]);
			log("Fourth Char: ",addr[3]);
			log("Fifth Char: ",addr[4]);
		}

		env.rm().detach(addr);
	}

	log("--------------------------------------------------------------------------");
	log("All dataspaces have been processed!");


}
