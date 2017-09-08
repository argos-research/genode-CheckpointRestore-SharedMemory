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



Genode::size_t Component::stack_size() { return 16*1024; }

void Component::construct(Genode::Env &env)
{
	//enter_kdebug("before restore");
	using namespace Genode;

	log("\033[32mAffinity space of child validator env: ",env.cpu().affinity_space().width(), " X ", env.cpu().affinity_space().height()," total:",env.cpu().affinity_space().total(),"\033[0m");
	log("Child validator up and running");


}
