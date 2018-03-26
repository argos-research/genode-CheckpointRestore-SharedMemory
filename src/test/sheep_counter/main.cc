/*
 * \brief  Testprogram which just counts sheeps
 * \author Denis Huber
 * \date   2016-08-04
 *
 * This program is a target for the rtcr service. It counts sheep,
 * prints the number of the sheep and goes to sleep between each 
 * iteration. This component will be checkpointed serialized, 
 * (transfered), deserialized, and restored. It does not know that it is
 * being checkpointed.
 */

#include <base/component.h>
#include <timer_session/connection.h>
#include <base/log.h>
#include <rm_session/connection.h>
#include <region_map/client.h>

//#define VERBOSE_REGISTER_DEBUG

Genode::size_t Component::stack_size() { return 16*1024; }

void Component::construct(Genode::Env &env)
{
	//enter_kdebug("before restore");
	using namespace Genode;
	log("Creating Timer session.");
	Timer::Connection timer(env);

	log("Allocating and attaching memory and its dataspace.");
	Dataspace_capability ds_cap = env.ram().alloc(4096);

#ifdef VERBOSE_REGISTER_DEBUG
	uint32_t stack_regs[16] = {0};
#endif /* VERBOSE_REGISTER_DEBUG */

	Genode::uint8_t* ds_addr = env.rm().attach(ds_cap);

	addr_t var_base_addr = 0x54;
	uint16_t &n = *(uint16_t*) (ds_addr + var_base_addr + 2);
	uint32_t &k = *(uint32_t*) (ds_addr + var_base_addr + 28);

	n=0;
	k=0xBEAD;
	size_t time_start;
	size_t time_end;

	while(1)
	{
		//Use busy loop instead of timer;
		//Pause/Resume does not work reliably with timer
		for(long long unsigned volatile busy = 0; busy <= 0x2FFFFFF; busy++)
		{}

		time_start = timer.elapsed_ms();
		n++;
		time_end = timer.elapsed_ms();
		k = n*0x10;
        log(Genode::Hex(n), " sheep. *10: ", Genode::Hex(k), ", Time for n++ (read, increment, write): ", time_end-time_start, "ms");

#ifdef VERBOSE_REGISTER_DEBUG
		register unsigned r0 asm("r0");
		register unsigned r1 asm("r1");
		register unsigned r2 asm("r2");
		register unsigned r3 asm("r3");
		register unsigned r4 asm("r4");
		register unsigned r5 asm("r5");
		register unsigned r6 asm("r6");
		register unsigned r7 asm("r7");
		register unsigned r8 asm("r8");
		register unsigned r9 asm("r9");
		register unsigned r10 asm("r10");
		register unsigned r11 asm("r11");
		register unsigned r12 asm("r12");
		register unsigned r13 asm("r13");
		register unsigned r14 asm("r14");
		//register unsigned r15 asm("r15"); -> IP
		stack_regs[0] = r0;
		stack_regs[1] = r1;
		stack_regs[2] = r2;
		stack_regs[3] = r3;
		stack_regs[4] = r4;
		stack_regs[5] = r5;
		stack_regs[6] = r6;
		stack_regs[7] = r7;
		stack_regs[8] = r8;
		stack_regs[9] = r9;
		stack_regs[10] = r10;
		stack_regs[11] = r11;
		stack_regs[12] = r12;
		stack_regs[13] = r13;
		stack_regs[14] = r14;
		//stack_regs[15] = r15; -> IP
		for(int i = 0; i < 15; i++)
		{
			log("Reg ", i, ":\t", Genode::Hex(stack_regs[i]), ",\tdec:", stack_regs[i]);
		}
#endif /* VERBOSE_REGISTER_DEBUG */

	}

}
