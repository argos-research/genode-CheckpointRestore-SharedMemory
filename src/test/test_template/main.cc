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
	char buf[160];
	Genode::snprintf(buf, sizeof(buf), "ram_quota=%u, label=\"%s\"", 20*1024*sizeof(long), "abc");

	log((const char*)buf);

	log("Hello world!");
}
