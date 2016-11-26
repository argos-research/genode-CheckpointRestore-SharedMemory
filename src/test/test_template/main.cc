/*
 * \brief  Template test
 * \author Denis Huber
 * \date   2016-08-20
 */

/* Genode includes */
#include <base/env.h>
#include <base/log.h>
#include <base/component.h>
#include <base/snprintf.h>

using namespace Genode;

size_t Component::stack_size() { return 64*1024; }

void Component::construct(Genode::Env &env)
{
	log("Hello world!");
}
