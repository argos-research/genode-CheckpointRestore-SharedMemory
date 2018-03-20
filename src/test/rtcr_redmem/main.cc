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

		Target_child* child = new (heap) Target_child { env, heap, parent_services, "sheep_counter", granularity };
		child->start();

		Target_state ts(env, heap);
		Checkpointer ckpt(heap, *child, ts);
		timer.msleep(1000);
		ckpt.set_redundant_memory(true);

		child->resume();

		for (int i = 0; i < 3 ; i++) {

			timer.msleep(3000);

			ckpt.checkpoint();

		}
		timer.msleep(2000);

		ckpt.set_redundant_memory(false);

		timer.msleep(2000);

		Ram_dataspace_info* rdsi = child->ram().parent_state().ram_dataspaces.first();
		Designated_redundant_ds_info* drdsi = (Designated_redundant_ds_info*) rdsi->mrm_info->dd_infos.first();
		PINF("Found source DRDSI");

		child->pause();
		timer.msleep(2000);
		Target_child* child2 = new (heap) Target_child { env, heap, parent_services, "sheep_counter", granularity };
		Target_state ts2(env, heap);
		Checkpointer ckpt2(heap, *child2, ts2);

		child2->start();



		//Target_child child_restored { env, heap, parent_services, "sheepcount", granularity };
		//Restorer resto(heap, child_restored, ts);
		//child_restored.start();


		timer.msleep(1000);
		//ckpt2.set_redundant_memory(true);


		Ram_dataspace_info* rdsi2 = child2->ram().parent_state().ram_dataspaces.first();
		Designated_redundant_ds_info* drdsi2 = (Designated_redundant_ds_info*) rdsi2->mrm_info->dd_infos.first();
		PINF("Found destination DRDSI");
		addr_t primary_ds_loc_addr2 = env.rm().attach(drdsi2->cap);
		drdsi->copy_from_latest_checkpoint((void*)primary_ds_loc_addr2);
		env.rm().detach(primary_ds_loc_addr2);
		PINF("MEMORY RESTORED!");

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
