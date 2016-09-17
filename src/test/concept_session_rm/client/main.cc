/*
 * \brief  Target component
 * \author Denis Huber
 * \date   2016-09-17
 */

/* Genode includes */
#include <base/env.h>
#include <base/log.h>
#include <base/component.h>

using namespace Genode;

size_t Component::stack_size() { return 16*1024; }

void Component::construct(Genode::Env &env)
{
	log("Hello world!");
}
