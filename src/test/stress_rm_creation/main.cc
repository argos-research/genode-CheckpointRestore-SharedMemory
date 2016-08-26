/*
 * \brief  Creates region maps from a rm session until quota exceedes and needs to be upgraded
 * \author Denis Huber
 * \date   2016-08-20
 */

/* Genode includes */
#include <base/env.h>
#include <base/log.h>
#include <base/component.h>
#include <rm_session/connection.h>
#include <base/snprintf.h>

using namespace Genode;


size_t Component::stack_size() { return 64*1024; }

void Component::construct(Genode::Env &env)
{
	log("--- Rm-creation-stresser started ---");
	Rm_connection rm{env};

	for(unsigned int i = 0; i < 10; i++)
	{
		log("Round ", i);
		try
		{
			rm.create(4096);
		}
		catch(Region_map::Out_of_metadata)
		{
			log("Exception caught!");
			char buf[Parent::Session_args::MAX_SIZE];
			snprintf(buf, sizeof(buf), "ram_quota=%u", 64*1024);

			env.parent().upgrade(rm, buf);

			rm.create(4096);
		}
	}

	log("--- Rm-creation-stresser ended ---");
}
