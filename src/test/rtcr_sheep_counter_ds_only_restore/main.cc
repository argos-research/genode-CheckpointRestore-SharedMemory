/* Genode include */
#include <base/component.h>
#include <base/signal.h>
#include <base/sleep.h>
#include <base/log.h>
#include <timer_session/connection.h>

/* Rtcr includes */
#include "../../rtcr/target_child.h"
#include "../../rtcr/target_state.h"
#include "../../rtcr/checkpointer.h"
#include "../../rtcr/restorer.h"

namespace Rtcr {
	class Main;
}

class Rtcr::Main
{
	enum { ROOT_STACK_SIZE = 16*1024 };
	Genode::Env              &env;
	Genode::Heap              heap            { env.ram(), env.rm() };
	Genode::Service_registry  parent_services { };

public:
	Main(Genode::Env &env_) : env(env_)
	{
		using namespace Genode;
		Timer::Connection timer { env };

		const size_t counter_dataspace_size = 5 * 4096;
		const size_t granularity = 1;

		Target_child target_child_checkpointed { env, heap, parent_services, "sheep_counter_extended", granularity };
		target_child_checkpointed.start();

		timer.msleep(5000);

		log("Checkpoint! --------------------------------------------------");

		Target_state target_state(env, heap);
		Checkpointer ckpt(heap, target_child_checkpointed, target_state);
		ckpt.checkpoint();

		log("Checkpoint complete! --------------------------------------------------");

		timer.msleep(2000);
		log("Checkpoint! --------------------------------------------------");
		ckpt.checkpoint();
		log("Checkpoint complete! --------------------------------------------------");

		// print checkpointed RAM session data and find info for dataspace with matching size
		log("Stored data of the RAM service in target_state:");
		Stored_ram_session_info *stored_ram_session_info = target_state._stored_ram_sessions.first();
		Stored_ram_dataspace_info *counter_stored_ram_dataspace_info = nullptr;

		do {
			log("Stored_ram_session_info at ", stored_ram_session_info, *stored_ram_session_info);
			Stored_ram_dataspace_info *stored_ram_dataspace_info =
					stored_ram_session_info->stored_ramds_infos.first();

			do {
				log("Stored_ram_dataspace_info at ", stored_ram_dataspace_info, *stored_ram_dataspace_info);
				if(stored_ram_dataspace_info->size == counter_dataspace_size)
					counter_stored_ram_dataspace_info = stored_ram_dataspace_info;
			} while((stored_ram_dataspace_info = stored_ram_dataspace_info->next()) != nullptr);
		} while((stored_ram_session_info = stored_ram_session_info->next()) != nullptr);

		if(!counter_stored_ram_dataspace_info) {
			log("Error: couldn't find dataspace with a size of ", counter_dataspace_size, "!");
			return;
		}

		timer.msleep(3000);
		target_child_checkpointed.pause();
		log("Checkpointed child paused.");

		log("Restore!    --------------------------------------------------");
		Target_child target_child_restored { env, heap, parent_services, "sheep_counter_extended", 0 };
		log("Starting new child...");
		target_child_restored.start();
		log("Waiting for child to attach dataspace...");

		const Ram_dataspace_capability *restored_child_ram_dataspace_capability = nullptr;

		while(!restored_child_ram_dataspace_capability) {
			restored_child_ram_dataspace_capability =
					get_Ram_dataspace_capability_with_size(counter_dataspace_size,
							target_child_restored.custom_services().ram_root);
		}

		// wait a bit to make sure that the sheep counter has initialized it's memory
		timer.msleep(10);

		log("Copying memory...");
		target_child_restored.pause();
		void *source_memory = env.rm().attach(counter_stored_ram_dataspace_info->memory_content);
		void *dest_memory = env.rm().attach(*restored_child_ram_dataspace_capability);
		memcpy(dest_memory, source_memory, counter_dataspace_size);
		log("Restore complete.");
		target_child_restored.resume();

		log("Main: End");
		Genode::sleep_forever();
	}

	const Genode::Ram_dataspace_capability *get_Ram_dataspace_capability_with_size(Genode::size_t size, Ram_root *ram_root) {
		using namespace Genode;
		Ram_session_component *ram_session_component = ram_root->session_infos().first();

		do {
			Ram_session_info &ram_session_info = ram_session_component->parent_state();
			Ram_dataspace_info *ram_dataspace_info = ram_session_info.ram_dataspaces.first();

			do {
				if(ram_dataspace_info->size == size) {
					return &(ram_dataspace_info->cap);
				}
			} while((ram_dataspace_info = ram_dataspace_info->next()) != nullptr);
		} while((ram_session_component = ram_session_component->next()) != nullptr);

		return 0;
	}
};

Genode::size_t Component::stack_size() { return 32*1024; }

void Component::construct(Genode::Env &env)
{
	static Rtcr::Main main(env);
}
