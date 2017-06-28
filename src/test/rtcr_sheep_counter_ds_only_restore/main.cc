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
	struct Main;
}

struct Rtcr::Main
{
	enum { ROOT_STACK_SIZE = 16*1024 };
	Genode::Env              &env;
	Genode::Heap              heap            { env.ram(), env.rm() };
	Genode::Service_registry  parent_services { };

	Main(Genode::Env &env_) : env(env_)
	{
		using namespace Genode;

		Timer::Connection timer { env };

		Target_child target_child_checkpointed { env, heap, parent_services, "sheep_counter", 0 };
		target_child_checkpointed.start();

		printRamInterceptionInfo(target_child_checkpointed.custom_services().ram_root);
		timer.msleep(5000);
		printRamInterceptionInfo(target_child_checkpointed.custom_services().ram_root);

		target_child_checkpointed.pause();
		log("Checkpoint! --------------------------------------------------");
		const size_t dataspace_size = 4096;
		const Ram_dataspace_capability *cp_child_ram_dataspace_capability =
				get_Ram_dataspace_capability_with_size(dataspace_size,
					 																		 target_child_checkpointed.custom_services().ram_root);

		if(!cp_child_ram_dataspace_capability) {
			log("Error: couldn't find dataspace to checkpoint!");
			return;
		}

		void *source_memory = env.rm().attach(*cp_child_ram_dataspace_capability);
		Dataspace_capability intermediate_dataspace_cap = env.ram().alloc(dataspace_size);
		void *intermediate_memory = env.rm().attach(intermediate_dataspace_cap);
		memcpy(intermediate_memory, source_memory, dataspace_size);
		log("Checkpoint complete.");
		target_child_checkpointed.resume();
		timer.msleep(3000);
		target_child_checkpointed.pause();
		log("Checkpointed child paused.");

		log("Restore!    --------------------------------------------------");
		Target_child target_child_restored { env, heap, parent_services, "sheep_counter", 0 };
		log("Starting new child...");
		target_child_restored.start();
		log("Waiting for child to attach dataspace...");

		const Ram_dataspace_capability *restored_child_ram_dataspace_capability = nullptr;

		while(!restored_child_ram_dataspace_capability) {
			restored_child_ram_dataspace_capability =
					get_Ram_dataspace_capability_with_size(dataspace_size,
						 																		 target_child_restored.custom_services().ram_root);
		}

		log("Copying memory...");

		target_child_restored.pause();
		void *dest_memory = env.rm().attach(*restored_child_ram_dataspace_capability);
		memcpy(dest_memory, intermediate_memory, dataspace_size);
		log("Restore complete.");
		target_child_restored.resume();

		log("Main: End");
		Genode::sleep_forever();
	}

	void printRamInterceptionInfo(Ram_root *ram_root) {
		using namespace Genode;

		log("printRamInterceptionInfo()");
		Ram_session_component *ram_session_component = ram_root->session_infos().first();

		do {
			log("Ram_session_component at ", ram_session_component);
			Ram_session_info &ram_session_info = ram_session_component->parent_state();
			Ram_dataspace_info *ram_dataspace_info = ram_session_info.ram_dataspaces.first();

			do {
				log("Ram_dataspace_info at ", ram_dataspace_info, " with size ", ram_dataspace_info->size);
			} while((ram_dataspace_info = ram_dataspace_info->next()) != nullptr);
		} while((ram_session_component = ram_session_component->next()) != nullptr);
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
