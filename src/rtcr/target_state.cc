/*
 * \brief  Target's state
 * \author Denis Huber
 * \date   2016-10-25
 */

#include "target_state.h"

using namespace Rtcr;


Target_state::Target_state(Genode::Ram_session &ram, Genode::Allocator &alloc)
:
	_ram   (ram),
	_alloc (alloc)
{ }


Target_state::~Target_state()
{
// TODO delete all list elements
}


void Target_state::print(Genode::Output &output) const
{
	using Genode::Hex;
	using Genode::print;

	Genode::print(output, "##########################\n");
	Genode::print(output, "###    Target_state    ###\n");
	Genode::print(output, "##########################\n");

	// PD session
	{
		Genode::print(output, "PD sessions:\n");
		Stored_pd_session_info const *pd_info = _stored_pd_sessions.first();
		if(!pd_info) Genode::print(output, " <empty>\n");
		while(pd_info)
		{
			print(output, " ", *pd_info, "\n");

			// Signal contexts
			print(output, "  Signal contexts:\n");
			Stored_signal_context_info const *context_info = pd_info->stored_context_infos.first();
			if(!context_info) print(output, "   <empty>\n");
			while(context_info)
			{
				print(output, "   ", *context_info, "\n");
				context_info = context_info->next();
			}

			// Signal sources
			print(output, "  Signal sources:\n");
			Stored_signal_source_info const *source_info = pd_info->stored_source_infos.first();
			if(!source_info) print(output, "   <empty>\n");
			while(source_info)
			{
				print(output, "   ", *source_info, "\n");
				source_info = source_info->next();
			}

			// Address space
			Stored_region_map_info const &region_map_info = pd_info->stored_address_space;
			print(output, "  Address space: ", region_map_info,"\n");
			Stored_attached_region_info const *attached_info = region_map_info.stored_attached_region_infos.first();
			if(!attached_info) print(output, "   <empty>\n");
			while(attached_info)
			{
				print(output, "   ", *attached_info, "\n");
				attached_info = attached_info->next();
			}

			// Stack area
			region_map_info = pd_info->stored_stack_area;
			print(output, "  Stack area: ", region_map_info,"\n");
			attached_info = region_map_info.stored_attached_region_infos.first();
			if(!attached_info) print(output, "   <empty>\n");
			while(attached_info)
			{
				print(output, "   ", *attached_info, "\n");
				attached_info = attached_info->next();
			}

			// Linker area
			region_map_info = pd_info->stored_linker_area;
			print(output, "  Linker area: ", region_map_info,"\n");
			attached_info = region_map_info.stored_attached_region_infos.first();
			if(!attached_info) print(output, "   <empty>\n");
			while(attached_info)
			{
				print(output, "   ", *attached_info, "\n");
				attached_info = attached_info->next();
			}

			pd_info = pd_info->next();
		}
	}
	// CPU session
	{
		print(output, "CPU sessions:\n");
		Stored_cpu_session_info const *cpu_info = _stored_cpu_sessions.first();
		if(!cpu_info) print(output, " <empty>\n");
		while(cpu_info)
		{
			print(output, " ", *cpu_info, "\n");

			Stored_cpu_thread_info const *cpu_thread_info = cpu_info->stored_cpu_thread_infos.first();
			if(!cpu_thread_info) print(output, "  <empty>\n");
			while(cpu_thread_info)
			{
				print(output, "  ", *cpu_thread_info, "\n");
				cpu_thread_info = cpu_thread_info->next();
			}

			cpu_info = cpu_info->next();
		}
	}
	// RAM session
	{
		Genode::print(output, "RAM sessions:\n");
		Stored_ram_session_info const *ram_info = _stored_ram_sessions.first();
		if(!ram_info) print(output, " <empty>\n");
		while(ram_info)
		{
			print(output, " ", *ram_info, "\n");

			Stored_ram_dataspace_info const *ramds_info = ram_info->stored_ramds_infos.first();
			if(!ramds_info) print(output, "  <empty>\n");
			while(ramds_info)
			{
				print(output, "  ", *ramds_info, "\n");
				ramds_info = ramds_info->next();
			}

			ram_info = ram_info->next();
		}
	}
	// ROM sessions
	{
		Genode::print(output, "ROM sessions:\n");
		Stored_rom_session_info const *rom_info = _stored_rom_sessions.first();
		if(!rom_info) Genode::print(output, " <empty>\n");
		while(rom_info)
		{
			Genode::print(output, " ", *rom_info, "\n");
			rom_info = rom_info->next();
		}
	}
	// RM sessions
	{
		Genode::print(output, "RM sessions:\n");
		Stored_rm_session_info const *rm_info = _stored_rm_sessions.first();
		if(!rm_info) Genode::print(output, " <empty>\n");
		while(rm_info)
		{
			Genode::print(output, " ", *rm_info, "\n");
			Stored_region_map_info const *region_map_info = rm_info->stored_region_map_infos.first();
			if(!region_map_info) Genode::print(output, "  <empty>\n");
			while(region_map_info)
			{
				Genode::print(output, "  ", *region_map_info, "\n");
				Stored_attached_region_info const *attached_info =
						region_map_info->stored_attached_region_infos.first();
				if(!attached_info) Genode::print(output, "   <empty>\n");
				while(attached_info)
				{
					Genode::print(output, "   ", *attached_info, "\n");
					attached_info = attached_info->next();
				}
				region_map_info = region_map_info->next();
			}
			rm_info = rm_info->next();
		}
	}
	// LOG sessions
	{
		Genode::print(output, "LOG sessions:\n");
		Stored_log_session_info const *log_info = _stored_log_sessions.first();
		if(!log_info) Genode::print(output, " <empty>\n");
		while(log_info)
		{
			Genode::print(output, " ", *log_info, "\n");
			log_info = log_info->next();
		}
	}
	// Timer sessions
	{
		Genode::print(output, "Timer sessions:\n");
		Stored_timer_session_info const *timer_info = _stored_timer_sessions.first();
		if(!timer_info) Genode::print(output, " <empty>\n");
		while(timer_info)
		{
			Genode::print(output, " ", *timer_info, "\n");
			timer_info = timer_info->next();
		}
	}
}

