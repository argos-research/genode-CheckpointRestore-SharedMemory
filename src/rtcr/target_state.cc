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

	Genode::print(output, "##########################\n");
	Genode::print(output, "###    Target_state    ###\n");
	Genode::print(output, "##########################\n");

	// RM sessions
	{
		Genode::print(output, "RM sessions:\n");
		const Stored_rm_session_info *rm_info = _stored_rm_sessions.first();
		if(!rm_info) Genode::print(output, " <empty>\n");
		while(rm_info)
		{
			Genode::print(output, " ", *rm_info, "\n");
			const Stored_region_map_info *region_map_info = rm_info->stored_region_map_infos.first();
			if(!region_map_info) Genode::print(output, "  <empty>\n");
			while(region_map_info)
			{
				Genode::print(output, "  ", *region_map_info, "\n");
				const Stored_attached_region_info *attached_info =
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
		const Stored_log_session_info *info = _stored_log_sessions.first();
		if(!info) Genode::print(output, " <empty>\n");
		while(info)
		{
			Genode::print(output, " ", *info, "\n");
			info = info->next();
		}
	}
	// Timer sessions
	{
		Genode::print(output, "Timer sessions:\n");
		const Stored_timer_session_info *info = _stored_timer_sessions.first();
		if(!info) Genode::print(output, " <empty>\n");
		while(info)
		{
			Genode::print(output, " ", *info, "\n");
			info = info->next();
		}
	}
	// PD session
	{
		Genode::print(output, "PD session:\n");
		Genode::print(output, _stored_pd_session, "\n");

		// Signal contexts
		Genode::print(output, " contexts:\n");
		const Stored_signal_context_info *context_info =
				_stored_pd_session.stored_context_infos.first();
		if(!context_info) Genode::print(output, " <empty>\n");
		while(context_info)
		{
			Genode::print(output, "  ", *context_info, "\n");
			context_info = context_info->next();
		}

		// Signal sources
		Genode::print(output, " sources:\n");
		const Stored_signal_source_info *source_info =
				_stored_pd_session.stored_source_infos.first();
		if(!source_info) Genode::print(output, " <empty>\n");
		while(source_info)
		{
			Genode::print(output, "  ", *source_info, "\n");
			source_info = source_info->next();
		}

		// Address space
		Genode::print(output, " address space:\n");
		Genode::print(output, _stored_pd_session.stored_address_space, "\n");
		const Stored_attached_region_info *as_info =
				_stored_pd_session.stored_address_space.stored_attached_region_infos.first();
		if(!as_info) Genode::print(output, " <empty>\n");
		while(as_info)
		{
			Genode::print(output, "  ", *as_info, "\n");
			as_info = as_info->next();
		}

		// Stack area
		Genode::print(output, " stack area:\n");
		Genode::print(output, _stored_pd_session.stored_stack_area, "\n");
		const Stored_attached_region_info *sa_info =
				_stored_pd_session.stored_stack_area.stored_attached_region_infos.first();
		if(!sa_info) Genode::print(output, " <empty>\n");
		while(sa_info)
		{
			Genode::print(output, "  ", *sa_info, "\n");
			sa_info = sa_info->next();
		}

		// Linker area
		Genode::print(output, " linker area:\n");
		Genode::print(output, _stored_pd_session.stored_linker_area, "\n");
		const Stored_attached_region_info *la_info =
				_stored_pd_session.stored_linker_area.stored_attached_region_infos.first();
		if(!la_info) Genode::print(output, " <empty>\n");
		while(la_info)
		{
			Genode::print(output, "  ", *la_info, "\n");
			la_info = la_info->next();
		}
	}
	// CPU session
	{
		Genode::print(output, "CPU session:\n");
		Genode::print(output, _stored_cpu_session, "\n");
		const Stored_thread_info *info = _stored_cpu_session.stored_thread_infos.first();
		if(!info) Genode::print(output, " <empty>\n");
		while(info)
		{
			Genode::print(output, " ", *info, "\n");
			info = info->next();
		}
	}
	// RAM session
	{
		Genode::print(output, "RAM session:\n");
		Genode::print(output, _stored_ram_session, "\n");
		const Ref_badge *info = _stored_ram_session.ref_badge_infos.first();
		if(!info) Genode::print(output, " <empty\n>");
		while(info)
		{
			Genode::print(output, " ", *info, "\n");
			info = info->next();
		}
	}
	// Dataspaces
	{
		Genode::print(output, "Dataspaces:\n");
		const Stored_dataspace_info *info = _stored_dataspaces.first();
		if(!info) Genode::print(output, " <empty>\n");
		while(info)
		{
			Genode::print(output, " ", *info, "\n");
			info = info->next();
		}
	}
}

