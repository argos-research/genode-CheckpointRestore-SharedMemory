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
#include <base/child.h>
//#include <base/internal/elf.h>
#include <../src/include/base/internal/elf.h>
//#include <../src/include/base/internal/elf_format.h>
#include <timer_session/connection.h>

/* Rtcr includes */
#include "../../rtcr/target_child.h"
#include "../../rtcr/target_state.h"
#include "../../rtcr/checkpointer.h"
#include "../../rtcr/restorer.h"

namespace Rtcr {
struct Main;
}

struct Rtcr::Main {
	enum {
		ROOT_STACK_SIZE = 16 * 1024
	};
	Genode::Env &env;
	Genode::Heap heap { env.ram(), env.rm() };
	Genode::Service_registry parent_services { };

	Main(Genode::Env &env_) :
			env(env_) {
		using namespace Genode;

		Timer::Connection timer { env };

		const Genode::size_t granularity = Target_child::GRANULARITY_REDUNDANT_MEMORY;

		Target_child* child = new (heap) Target_child { env, heap, parent_services, "sheepcount", granularity };
		child->start();

		Target_state ts(env, heap);
		Checkpointer ckpt(heap, *child, ts);
		timer.msleep(1000);
		ckpt.activate_redundant_memory();

		child->resume();

		for (int i = 0; i < 2 ; i++) {

			timer.msleep(3000);

			ckpt.checkpoint();

			//child->resume();

		}
		timer.msleep(2000);


		child->exit(0);
		child->pause();
		Genode::destroy(heap,child);

		timer.msleep(2000);


		Ram_dataspace_info* rdsi = child->ram().parent_state().ram_dataspaces.first();
		PINF("found RDSI");
		Designated_redundant_ds_info* drdsi = (Designated_redundant_ds_info*) rdsi->mrm_info->dd_infos.first();
		addr_t primary_ds_loc_addr = env.rm().attach(drdsi->cap);
		memcpy((char*)primary_ds_loc_addr,(char*)drdsi->get_first_checkpoint()->get_address(),drdsi->size);
		env.rm().detach(primary_ds_loc_addr);


		Target_child child_restored { env, heap, parent_services, "sheepcount", granularity };
		//Restorer resto(heap, child_restored, ts);
		child_restored.start();


		timer.msleep(3000);




		log("The End");
		Genode::sleep_forever();
	}
};

Genode::size_t Component::stack_size() {
	return 32 * 1024;
}

void Component::construct(Genode::Env &env) {
	static Rtcr::Main main(env);
}
