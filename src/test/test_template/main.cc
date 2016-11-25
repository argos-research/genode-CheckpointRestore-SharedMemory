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

struct Base
{
	void print(Genode::Output &output) const
	{
		Genode::print(output, "Base");
	}
};

struct Derived : Base
{
	void print(Genode::Output &output) const
	{
		Base::print(output);
		Genode::print(output, " Derived");
	}
};


size_t Component::stack_size() { return 64*1024; }

void Component::construct(Genode::Env &env)
{
	Derived d;

	log(d);

	log("Hello world!");
}
