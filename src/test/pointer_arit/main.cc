/*
 * \brief  Pointer arithmetic test
 * \author Denis Huber
 * \date   2016-08-31
 */

/* Genode includes */
#include <base/env.h>
#include <base/log.h>
#include <base/component.h>
#include <base/attached_ram_dataspace.h>
#include <util/general.h>

using namespace Genode;

size_t Component::stack_size() { return 64*1024; }

void Component::construct(Genode::Env &env)
{
	log("--- pointer-test started ---");
	Attached_ram_dataspace ds {env.ram(), env.rm(), 4096};

	// ptr is the main pointer, it shall point to the next free location in the dataspace
	void* ptr = ds.local_addr<void>();

	// print_ptr is a pointer to the location that shall be printed (const value)
	void* print_ptr = ptr;

	log("Start: ptr  = ", ptr);

	// Data to store to ptr's location
	addr_t data0 = 0x1234affe;
	addr_t data1 = 0xbabe5678;

	Rtcr::dump_mem(print_ptr, 0x10);

	/*
	 * Special pointer that has the same type as the data.
	 * void* cannot be used in pointer arithmetic, thus we need a typed pointer (other than void)
	 */
	addr_t* spec_ptr = static_cast<addr_t*>(ptr);

	// Post-increment: Increment spec_ptr, but use the lvalue before the increment
	*(spec_ptr++) = data0;
	*(spec_ptr++) = data1;

	ptr = spec_ptr;

	Rtcr::dump_mem(print_ptr, 0x10);

	log("End: ptr = ", ptr);

	log("--- pointer-test ended ---");
}
