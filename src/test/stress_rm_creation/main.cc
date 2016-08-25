/*
 * \brief  Creates managed dataspaces with ram dataspaces
 * \author Denis Huber
 * \date   2016-08-20
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
	log("--- Rm-creation-stresser started ---");
	Rm_connection rm_con{env};

	//env.parent().upgrade(rm_con.cap(), "ram_quota=131072");

	for(unsigned int i = 0; i < 10; i++)
	{
		log("Round ", i);
		try
		{
			rm_con.create(4096);
		}
		catch(Allocator::Out_of_memory)
		{
			log("Exception caught!");
		}
	}

	log("--- Rm-creation-stresser ended ---");
}
