/*
 * \brief  Random program for testing Genode's functionalities
 * \author Denis Huber
 * \date   2016-08-13
 */

/* Genode includes */
#include <base/env.h>
#include <base/log.h>
#include <base/component.h>
#include <cpu_thread/client.h>
#include <base/semaphore.h>

/* Rtcr includes */
#include <util/general.h>

using namespace Genode;


size_t Component::stack_size() { return 64*1024; }

void Component::construct(Genode::Env &env)
{

}
