/*
 * \brief  Rtcr session driver
 * \author Denis Huber
 * \date   2016-08-20
 */

/* Genode includes */
#include <base/env.h>
#include <base/log.h>
#include <base/component.h>

/* Rtcr includes */
#include <rtcr_session/connection.h>

using namespace Genode;

size_t Component::stack_size() { return 64*1024; }

void Component::construct(Genode::Env &env)
{
	log("--- Rtcr-driver started ---");

	Rtcr::Connection rtcr { env };
	rtcr.checkpoint("sheep_counter");
	rtcr.restore("sheep_counter");

	log("--- Rtcr-driver ended ---");
}
