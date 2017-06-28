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
#include "../../rtcr/target_state.h"
#include "../../rtcr/checkpointer.h"
#include "../../rtcr/restorer.h"
#include "../../rtcr/multicore/validator_session.h"
#include "../../rtcr/multicore/validator_session/validator_session.h"
#include "../../rtcr/util/sizes.h"

namespace Rtcr {
	struct Main;
}

namespace Fiasco {
#include <l4/sys/kdebug.h>
}

struct Rtcr::Main
{


	Genode::Env              &env;
	Genode::Heap              heap            { env.ram(), env.rm() };
	Genode::Service_registry  parent_services { };

	Main(Genode::Env &env_) : env(env_)
	{
		using namespace Genode;

		Entrypoint ep(env, EP_STACK_SIZE, "rtcr_ep");

		Timer::Connection timer { env };

		Target_state state(env, heap);

		Validator_root val_root(env, heap, ep, state, DS_ALLOC_SIZE);
		Genode::Local_service val_service(Validator_session::service_name(), &val_root);
		parent_services.insert(&val_service);


		Target_child child_counter { env, heap, parent_services, "rtcr_mc_counter", 0 };
		log("\033[32mChild_counter object created\033[0m");
		child_counter.start();
		log("\033[32mChild_counter started\033[0m");

		timer.msleep(3000);

		// \033[32m \033[0m

		Checkpointer ckpt(heap, child_counter, state);
		ckpt.checkpoint();

		timer.msleep(3000);

		Target_child child_validator { env, heap, parent_services, "rtcr_mc_validator", 0 };
		log("\033[32mChild_validator object created\033[0m");
		child_validator.start();
		log("\033[32mChild_validator started\033[0m");

//		Restorer resto(heap, child_restored, ts);
//		child_restored.start(resto);

		//log("The End");
		Genode::sleep_forever();
	}
};

Genode::size_t Component::stack_size() { return 32*1024; }

void Component::construct(Genode::Env &env)
{
	static Rtcr::Main main(env);
}
