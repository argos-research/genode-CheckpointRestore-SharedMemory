/*
 * fault_handler.cpp
 *
 *  Created on: Jan 25, 2018
 *      Author: josef
 */
#include "fault_handler.h"
#include "../intercept/cpu_session.h"
#include "../arm_v7a/instruction.h"

using namespace Rtcr;

Managed_region_map_info *Fault_handler::_find_faulting_mrm_info()
{
	Genode::Region_map::State state;
	Managed_region_map_info *result_info = nullptr;

	Ram_dataspace_info *ramds_info = _ramds_infos.first();
	while(ramds_info && !result_info)
	{
		if(!ramds_info->mrm_info)
			continue;

		Genode::Region_map_client rm_client(ramds_info->mrm_info->region_map_cap);

		if(rm_client.state().type != Genode::Region_map::State::READY)
		{
			result_info = ramds_info->mrm_info;
			break;
		}

		ramds_info = ramds_info->next();
	}

	return result_info;
}

void to_big_endian(unsigned& le)
{
	unsigned swapped = ((le>>24)&0xff) | // move byte 3 to byte 0
	                    ((le<<8)&0xff0000) | // move byte 1 to byte 2
	                    ((le>>8)&0xff00) | // move byte 2 to byte 1
	                    ((le<<24)&0xff000000); // byte 0 to byte 3
	le=swapped;
}

void Fault_handler::_handle_fault()
{
	// Find faulting Managed_region_info
	Managed_region_map_info *faulting_mrm_info = _find_faulting_mrm_info();


	// Get state of faulting Region_map
	Genode::Region_map::State state = Genode::Region_map_client{faulting_mrm_info->region_map_cap}.state();

	if(verbose_debug)
	{
	Genode::log("Handle fault: Region map ",
			faulting_mrm_info->region_map_cap, " state is ",
			state.type == Genode::Region_map::State::READ_FAULT  ? "READ_FAULT"  :
			state.type == Genode::Region_map::State::WRITE_FAULT ? "WRITE_FAULT" :
			state.type == Genode::Region_map::State::EXEC_FAULT  ? "EXEC_FAULT"  : "READY",
			" pf_addr=", Genode::Hex(state.addr), " ip: ", Genode::Hex(state.pf_ip));
	}

	// Find dataspace which contains the faulting address

	Designated_dataspace_info *dd_info = faulting_mrm_info->dd_infos.first();
	if(dd_info) dd_info = dd_info->find_by_addr(state.addr);


	// Check if a dataspace was found
	if(!dd_info)
	{
		Genode::warning("No designated dataspace for addr = ", state.addr,
				" in Region_map ", faulting_mrm_info->region_map_cap);
		return;
	}


	// Find thread which caused the fault

	Cpu_thread_component * cpu_thread = nullptr;
	Cpu_session_component* c = Cpu_session_component::current_session;
	while (c) {
		// Iterate through every CPU thread
		cpu_thread =
				c->parent_state().cpu_threads.first();
		int j = 0;
		while (cpu_thread) {
			// Pause the CPU thread
			//if(cpu_thread->state().unresolved_page_fault)
			{
				if(state.pf_ip == cpu_thread->state().ip)
				{
					PINF("Page-faulting Thread: %i, Pagefault: %i, IP: %lx", j,
						cpu_thread->state().unresolved_page_fault,
						cpu_thread->state().ip);
					goto found_thread;
				}
				j++;
			}
			cpu_thread = cpu_thread->next();
		}

		c = c->next();
	}

	found_thread:

	// Get copy of state
	Genode::Thread_state thread_state = cpu_thread->state();


	// Don't attach found dataspace to its designated address,
	// because we need to continue receiving pagefaults in order
	// to simulate them
	//dd_info->attach();
	//dd_info->detach();

	//TODO: simulate instruction





	addr_t inst_addr = state.pf_ip;
	unsigned instr = *((uint32_t* ) (elf_addr + elf_seg_offset + inst_addr - elf_seg_addr));
	PINF("Address: %lx, Offset: %lx, value: %x, target inst: %x",
			elf_seg_addr, elf_seg_offset, *((uint32_t* ) (elf_addr + elf_seg_offset)),
			*((uint32_t* ) (elf_addr + elf_seg_offset + inst_addr - elf_seg_addr)));

	/* decode the instruction and update state accordingly */
	bool writes = false;
	bool ldst = Instruction::load_store(instr, writes, state.format, state.reg);


	PINF("instruction: %x, %s, %s, %s, reg: %x", instr, ldst ? "LOADSTORE" : "OTHER",
			writes ? "store" : "load",
				state.format == Region_map::LSB8 ? "LSB8 " : (state.format == Region_map::LSB16 ? "LSB16" : "LSB32"), state.reg);

	size_t access_size =
			state.format == Region_map::LSB8 ? 1 : (state.format == Region_map::LSB16 ? 2 : 4);

	char* addr = _env.rm().attach(dd_info->cap);

	/* The address included in the pagefault report
	 * is 8-byte-aligned. In order to obtain the exact
	 * address of the memory access, we use the address
	 * included in the instruction.
	 */
	state.addr += ( instr % 8 );


	//addr_t* regs[15];
	//_get_register_mapping(regs,thread_state);


	if(!writes)
	{
		state.value = 0;
		memcpy(&state.value,addr + state.addr,access_size);
		PINF("Value at %lx: %x", state.addr, state.value);
		//thread_state.r3 =51;
		unsigned old_val = 0;
		thread_state.get_gpr(state.reg, old_val);
		PINF("Old register value: %x", old_val);
		unsigned const reg = state.reg;
		to_big_endian(state.value);
		thread_state.set_gpr(reg,state.value);
	}

	else
	{
		thread_state.get_gpr(state.reg, state.value);
		PINF("Old register value: %x", state.value);
		to_big_endian(state.value);
		memcpy(addr + state.addr,&state.value,access_size);
	}





	//value+=10;
	//memcpy(addr + state.addr,&value,sizeof(value));
	_env.rm().detach(addr);
	//dd_info->attach();



	// Increase instruction pointer (ip) by one word
	thread_state.ip += Instruction::size();
	// Write back modified state
	cpu_thread->state(thread_state);
	PINF("IP: %lx", thread_state.ip);

	//continue execution since we resolved the pagefault
	Genode::Region_map_client{faulting_mrm_info->region_map_cap}.processed(state);
}


Fault_handler::Fault_handler(Genode::Env &env, Genode::Signal_receiver &receiver,
		Genode::List<Ram_dataspace_info> &ramds_infos)
:
	Thread(env, "managed dataspace pager", 16*1024),
	_env(env),
	_receiver(receiver), _ramds_infos(ramds_infos)
{

    static Rom_connection rom("sheepcount");
	Dataspace_capability elf_ds = rom.dataspace();

	/* attach ELF locally */
	try {
		elf_addr = _env.rm().attach(elf_ds);
	} catch (Region_map::Attach_failed) {
		error("local attach of ELF executable failed");
		throw;
	}

	/* setup ELF object and read program entry pointer */
	Elf_binary elf(elf_addr);
	if (!elf.valid())
		PINF("Invalid binary");

	PINF("First 4 Bytes: %x", *(uint8_t* )elf_addr);

	addr_t entry = elf.entry();
	Elf_segment seg;
	for (unsigned n = 0; (seg = elf.get_segment(n)).valid(); ++n) {
		if (seg.flags().skip)
			continue;

		/* same values for r/o and r/w segments */
		elf_seg_addr = (addr_t) seg.start();
		elf_seg_size = seg.mem_size();

		bool parent_info = false;

		bool const write = seg.flags().w;
		bool const exec = seg.flags().x;
		if (!write) {
			/* read-only segment */

			if (seg.file_size() != seg.mem_size())
				warning("filesz and memsz for read-only segment differ");

			elf_seg_offset = seg.file_offset();
			if (exec)
			{
				/* Found the executable segment, break in order not
				 * to overwrite the addresses with other segments'.
				 */
				break;
			}
		}
	}




}


void Fault_handler::entry()
{
	while(true)
	{
		Genode::Signal signal = _receiver.wait_for_signal();
		for(unsigned int i = 0; i < signal.num(); ++i)
			_handle_fault();
	}
}
