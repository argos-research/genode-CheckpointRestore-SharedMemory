/*
 * \brief  Rtcr child creation
 * \author Denis Huber
 * \date   2016-08-26
 */

/* Genode include */
#include <base/component.h>
#include <base/signal.h>
#include <base/sleep.h>
#include <base/log.h>
#include <timer_session/connection.h>

/* Rtcr includes */
#include "../../rtcr/target_child.h"
#include "../../rtcr/util/general.h"

namespace Rtcr {
	struct Main;
}

struct Rtcr::Main
{
	enum { ROOT_STACK_SIZE = 16*1024 };
	Genode::Env             &env;
	Genode::Heap             md_heap         { env.ram(), env.rm() };
	Genode::Service_registry parent_services { };

	Main(Genode::Env &env_) : env(env_)
	{
		using namespace Genode;

		Target_child child { env, md_heap, parent_services, "sheep_counter", true };

		Timer::Connection timer {env};

		timer.msleep(2000);

		List<Attached_region_info> &ar_infos = child.pd().address_space_component().attached_regions();
		List<Managed_region_info>  &mr_infos = child.ram().managed_regions();

		Attached_region_info *ar_info = ar_infos.first();
		if(ar_info) ar_info = ar_info->find_by_addr(0x1000);
		if(ar_info)
			log("Found attached region at address ", Hex(ar_info->addr));
		else
		{
			log("Did not found attached region at address 0x1000");
			return;
		}

		Managed_region_info *mr_info = mr_infos.first();
		if(mr_info) mr_info = mr_info->find_by_cap(ar_info->ds_cap);
		if(mr_info)
			log("Found corresponding managed region");
		else
		{
			log("Did not found managed region for the attached region");
			return;
		}

		for(unsigned int i = 0; i < 100; ++i)
		{
			timer.msleep(4000);
			log("Pausing ", i);
			child.pause();

			log("Managed regions");
			print_managed_region_info_list(mr_infos);

			log("Detaching");
			for(Attachable_dataspace_info *ad_info = mr_info->ad_infos.first(); ad_info; ad_info = ad_info->next())
			{
				ad_info->detach();
			}

			Attachable_dataspace_info *ad_info = mr_info->ad_infos.first();
			unsigned int *addr = env.rm().attach(ad_info->ds_cap);
			log("Reading ", *addr, " sheeps");
			env.rm().detach(addr);

			child.resume();
			log("Resumed ", i);
		}

		Genode::sleep_forever();
	}
};

Genode::size_t Component::stack_size() { return 32*1024; }

void Component::construct(Genode::Env &env)
{
	static Rtcr::Main main(env);
}
