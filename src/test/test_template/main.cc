/*
 * \brief  Template test
 * \author Denis Huber
 * \date   2016-08-20
 */

/* Genode includes */
#include <base/env.h>
#include <base/log.h>
#include <base/component.h>
#include <util/list.h>

using namespace Genode;

template<typename T>
struct my_info : List<T>::Element
{
	my_info() { }

	void print(Genode::Output &output) const
	{
		using Genode::Hex;
		using Genode::print;

		print(output, "my_info");
	}
};

struct derived_info : my_info<derived_info>
{
	uint32_t x;

	derived_info() : x(0) { }

	derived_info *find_by_badge(Genode::uint16_t badge)
	{
		if(badge == x)
			return this;
		derived_info *info = next();
		return info ? info->find_by_badge(badge) : 0;
	}


	void print(Genode::Output &output) const
	{
		using Genode::Hex;
		using Genode::print;

		print(output, "derived_info ");
		my_info::print(output);
	}
};


size_t Component::stack_size() { return 64*1024; }

void Component::construct(Genode::Env &env)
{
	derived_info a;
	derived_info b;
	derived_info c;
	derived_info d;

	List<derived_info> l;
	l.insert(&a);
	l.insert(&b);
	l.insert(&c);

	a.find_by_badge(1000);

	log(d);

	log("Hello world!");
}
