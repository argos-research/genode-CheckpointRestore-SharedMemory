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

		Target_child child { env, md_heap, parent_services, "sheep_counter", 1 };

		Timer::Connection timer { env };
		timer.msleep(1000);

		log("Pausing");
		child.pause();

		log("Stack area");
		List<Attached_region_info> &stack_area = child.pd().stack_area_component().attached_regions();
		print_attached_region_info_list(stack_area);

		log("Managed dataspaces");
		List<Managed_region_info> &mr_list = child.ram().managed_regions();
		print_managed_region_info_list(mr_list);

		log("Detaching");

		for(Attached_region_info *ar_info = stack_area.first(); ar_info; ar_info = ar_info->next())
		{
			Managed_region_info *mr_info = mr_list.first()->find_by_cap(ar_info->ds_cap);

			for(Attachable_dataspace_info *ad_info = mr_info->attachable_dataspaces.first(); ad_info; ad_info = ad_info->next())
			{
				log("  dataspace (", ad_info->dataspace.local_name(),
						"|", Hex(ad_info->local_addr),
						"|", Hex(ad_info->size),
						"|", ad_info->attached?"a":"d", ")");

				if(ad_info->attached)
				{
					ad_info->detach();
				}
			}
		}

		log("Managed dataspaces");
		print_managed_region_info_list(mr_list);

		log("Resuming");
		child.resume();

		Genode::sleep_forever();
	}
};

Genode::size_t Component::stack_size() { return 32*1024; }

void Component::construct(Genode::Env &env)
{
	static Rtcr::Main main(env);
}
