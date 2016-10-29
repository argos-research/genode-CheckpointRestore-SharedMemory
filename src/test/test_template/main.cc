/*
 * \brief  Template test
 * \author Denis Huber
 * \date   2016-08-20
 */

/* Genode includes */
#include <base/env.h>
#include <base/log.h>
#include <base/component.h>

using namespace Genode;

size_t Component::stack_size() { return 64*1024; }

void Component::construct(Genode::Env &env)
{
	unsigned a = 42;
	unsigned *ptr = nullptr;
	bool b = ptr;
	log("Assigning nullptr: ", b);
	ptr = &a;
	b = ptr;
	log("Assigning 0x1000: ", b);

	log("Hello world!");
}
