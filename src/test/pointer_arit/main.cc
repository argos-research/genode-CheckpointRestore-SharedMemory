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

void dump_mem(const void *mem, unsigned int size)
{
	using namespace Genode;

	const char *p = reinterpret_cast<const char*>(mem);

	log("Block: [", Hex((addr_t)mem), ", ", Hex((addr_t)mem + (addr_t)size), ")");
	for(unsigned int i = 0; i < size/16+1; i++)
	{
		log(Hex(i*16, Hex::PREFIX, Hex::PAD),
				"  ", Hex(p[i*16+0],  Hex::OMIT_PREFIX, Hex::PAD), " ", Hex(p[i*16+1],  Hex::OMIT_PREFIX, Hex::PAD),
				" ",  Hex(p[i*16+2],  Hex::OMIT_PREFIX, Hex::PAD), " ", Hex(p[i*16+3],  Hex::OMIT_PREFIX, Hex::PAD),
				"  ", Hex(p[i*16+4],  Hex::OMIT_PREFIX, Hex::PAD), " ", Hex(p[i*16+5],  Hex::OMIT_PREFIX, Hex::PAD),
				" ",  Hex(p[i*16+6],  Hex::OMIT_PREFIX, Hex::PAD), " ", Hex(p[i*16+7],  Hex::OMIT_PREFIX, Hex::PAD),
				"  ", Hex(p[i*16+8],  Hex::OMIT_PREFIX, Hex::PAD), " ", Hex(p[i*16+9],  Hex::OMIT_PREFIX, Hex::PAD),
				" ",  Hex(p[i*16+10], Hex::OMIT_PREFIX, Hex::PAD), " ", Hex(p[i*16+11], Hex::OMIT_PREFIX, Hex::PAD),
				"  ", Hex(p[i*16+12], Hex::OMIT_PREFIX, Hex::PAD), " ", Hex(p[i*16+13], Hex::OMIT_PREFIX, Hex::PAD),
				" ",  Hex(p[i*16+14], Hex::OMIT_PREFIX, Hex::PAD), " ", Hex(p[i*16+15], Hex::OMIT_PREFIX, Hex::PAD));
	}

}

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

	dump_mem(print_ptr, 0x10);

	/*
	 * Special pointer that has the same type as the data.
	 * void* cannot be used in pointer arithmetic, thus we need a typed pointer (other than void)
	 */
	addr_t* spec_ptr = static_cast<addr_t*>(ptr);

	// Post-increment: Increment spec_ptr, but use the lvalue before the increment
	*(spec_ptr++) = data0;
	*(spec_ptr++) = data1;

	// Compute difference between both pointers in bytes
	size_t diff = spec_ptr - static_cast<addr_t*>(ptr);

	ptr = spec_ptr;

	dump_mem(print_ptr, 0x10);

	log("End: ptr = ", ptr);
	log("Stored bytes: ", diff);

	log("--- pointer-test ended ---");
}
