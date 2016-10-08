/*
 * \brief  Testing object with a custom print method for Genode::log
 * \author Denis Huber
 * \date   2016-10-07
 */

/* Genode includes */
#include <base/env.h>
#include <base/log.h>
#include <base/component.h>
#include <base/heap.h>

using namespace Genode;

namespace Rtcr {

	struct A : List<A>::Element
	{
		addr_t addr;
		size_t size;
		Native_capability cap;

		A(addr_t addr, size_t size, Native_capability cap)
		:
			addr(addr),
			size(size),
			cap(cap)
		{ }

		void print(Output &output) const
		{
			Genode::print(output, cap);
			Genode::print(output, " [");
			Genode::print(output, Hex(addr));
			Genode::print(output, ", ");
			Genode::print(output, Hex(addr + size));
			Genode::print(output, ")");
		}
	};

	struct B : List<B>::Element
	{
		unsigned var0;
		const char* var1;
	};

	void print(Output &output, const B &b)
	{
		Genode::print(output, b.var0, ", ", b.var1);
	}


	template<typename STRUCT_INFO>
	void print(Output &output, List<STRUCT_INFO> &infos)
	{
		STRUCT_INFO *info = infos.first();
		while(info)
		{
			print(output, *info);
			print(output, "\n");

			info = info->next();
		}
	}

}
size_t Component::stack_size() { return 32*1024; }

void Component::construct(Genode::Env &env)
{
	log("--- custom_print-test started ---");

	using namespace Rtcr;

	A a(0, 12*4096, env.ram_session_cap());
	log(a);

	B b;
	b.var0 = 1000;
	b.var1 = "hello print test!";
	log(b);

	List<A> a_list;
	Heap alloc(env.ram(), env.rm());
	for(unsigned i = 0; i < 5; ++i)
	{
		A *new_a = new (alloc) A(i*4096, 4096, Native_capability());
		a_list.insert(new_a);
	}

	log(a_list);

	log("--- custom_print-test ended ---");
}
