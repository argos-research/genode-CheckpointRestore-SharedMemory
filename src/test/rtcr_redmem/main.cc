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
#include "../../rtcr/util/simplified_managed_dataspace_info.h"

namespace Rtcr {
struct Main;
enum cr_type_t {full, incremental, redundant_memory};
constexpr cr_type_t cr_type = redundant_memory;
constexpr bool restore_memory_only = false;
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
		size_t time_start;
		size_t time_end;

		if(restore_memory_only && cr_type != redundant_memory)
		{
			PERR("Memory-only restore only supported on redundant memory.");
			Genode::sleep_forever();
			return;
		}

		const Genode::size_t granularity = cr_type == full ? 0 :
				cr_type == incremental ? 0x100 : Target_child::GRANULARITY_REDUNDANT_MEMORY;

		Target_child* child = new (heap) Target_child { env, heap, parent_services, "sheep_counter", granularity };
		Target_state ts(env, heap, cr_type == redundant_memory);
		Checkpointer ckpt(heap, *child, ts);
		child->start();

		timer.msleep(1000);

		if(cr_type == redundant_memory)
		{
			PINF("Enable redundant memory");
			ckpt.set_redundant_memory(true);
		}

		for (int i = 0; i < 3 ; i++) {

			timer.msleep(3000);

			PINF("Trigger checkpoint");
			time_start = timer.elapsed_ms();
			ckpt.checkpoint();
			time_end = timer.elapsed_ms();
			PINF("Time for checkpointing: %ums", time_end-time_start);
		}
		timer.msleep(2000);

		PINF("Pause target and create new process as restoration target");
		child->pause();
		Target_child* child_restored = new (heap) Target_child { env, heap, parent_services, "sheep_counter", granularity };
		Target_state ts_restored(env, heap, cr_type == redundant_memory);
		Checkpointer ckpt_restored(heap, *child_restored, ts_restored);

		if(!restore_memory_only)
		{
			Restorer resto(heap, *child_restored, ts);
			child_restored->start(resto);
			timer.msleep(2000);
			log("The End.");
			Genode::sleep_forever();
			return;
		}

		child_restored->start();
		timer.msleep(3000);

		// Manual memory-only restore

		PINF("Restore memory");
		/* Find the custom dataspace info with the snapshot inside:
		 * It's the ds that sheep_counter created last during runtime,
		 * so it's the first one in the list.
		 */
		Simplified_managed_dataspace_info* src_smdi = ts._managed_redundant_dataspaces.first();
		Simplified_managed_dataspace_info::Simplified_designated_ds_info* src_sddsi = src_smdi->designated_dataspaces.first();
		Designated_redundant_ds_info* src_drdsi = src_sddsi->redundant_memory;

		/* Now, locate the target ds of the new task.
		 * The order of ram_dataspaces is reversed,
		 * so this time we need the first element.
		 */
		Ram_dataspace_info* dst_rdsi = child_restored->ram().parent_state().ram_dataspaces.first();
		Designated_dataspace_info* dst_ddsi = dst_rdsi->mrm_info->dd_infos.first();
		addr_t primary_ds_loc_addr = env.rm().attach(dst_ddsi->cap);
		// Now we can do the memory restoration:
		src_drdsi->copy_from_latest_checkpoint((void*)primary_ds_loc_addr);
		env.rm().detach(primary_ds_loc_addr);

		timer.msleep(5000);

		log("The End.");
		Genode::sleep_forever();
	}
};

Genode::size_t Component::stack_size() {
	return 32 * 1024;
}

void Component::construct(Genode::Env &env) {
	static Rtcr::Main main(env);
}
