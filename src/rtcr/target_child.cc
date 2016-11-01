/*
 * \brief  Child creation
 * \author Denis Huber
 * \date   2016-08-04
 */

#include "target_child.h"

using namespace Rtcr;


Target_child::Resources::Resources(Genode::Env &env, Genode::Entrypoint &ep, Genode::Allocator &md_alloc,
		const char *name, Genode::size_t granularity)
:
	ep  (ep),
	pd  (env, md_alloc, ep, name),
	cpu (env, md_alloc, ep, pd.parent_cap(), name),
	ram (env, md_alloc, ep, name, granularity),
	rom (env, md_alloc, ep, name)
{
	ep.manage(pd);
	ep.manage(cpu);
	ep.manage(ram);
	ep.manage(rom);

	if(verbose_debug)
	{
		Genode::log("Managing PD  session ", pd.cap());
		Genode::log("Managing CPU session ", cpu.cap());
		Genode::log("Managing RAM session ", ram.cap());
		Genode::log("Managing ROM session ", rom.cap());
	}


	// Donate ram quota to child
	// TODO Replace static quota donation with the amount of quota, the child needs
	Genode::size_t donate_quota = 1024*1024;
	ram.ref_account(env.ram_session_cap());
	// Note: transfer goes directly to parent's ram session
	env.ram().transfer_quota(ram.parent_cap(), donate_quota);
}


Target_child::Resources::~Resources()
{
	ep.dissolve(rom);
	ep.dissolve(ram);
	ep.dissolve(cpu);
	ep.dissolve(pd);

	if(verbose_debug)
	{
		Genode::log("Dissolving PD  session ", pd.cap());
		Genode::log("Dissolving CPU session ", cpu.cap());
		Genode::log("Dissolving RAM session ", ram.cap());
		Genode::log("Dissolving ROM session ", rom.cap());
	}
}


Target_child::Target_child(Genode::Env &env, Genode::Allocator &md_alloc,
		Genode::Service_registry &parent_services, const char *name, Genode::size_t granularity)
:
	_name            (name),
	_env             (env),
	_md_alloc        (md_alloc),
	_resources_ep    (_env, 16*1024, "resources ep"),
	_child_ep        (_env, 16*1024, "child ep"),
	_granularity     (granularity),
	_restorer        (nullptr),
	_resources       (_env, _resources_ep, _md_alloc, _name.string(), _granularity),
	_initial_thread  (_resources.cpu, _resources.pd.cap(), _name.string()),
	_address_space   (_resources.pd.address_space()),
	_parent_services (parent_services),
	_local_services  (),
	_child_services  (),
	_rm_root         (nullptr),
	_log_root        (nullptr),
	_timer_root      (nullptr),
	_child           (nullptr)
{
	if(verbose_debug) Genode::log("\033[33m", __func__, "\033[0m(child=", _name.string(), ")");
}


Target_child::~Target_child()
{
	if(_child)
		Genode::destroy(_md_alloc, _child);

	if(verbose_debug) Genode::log("\033[33m", __func__, "\033[0m() ", _name.string());
}


void Target_child::start()
{
	if(verbose_debug) Genode::log("Target_child::\033[33m", __func__, "\033[0m()");

	_child = new (_md_alloc) Genode::Child(
			Genode::Rom_session_client{_resources.rom.parent_cap()}.dataspace(),
			Genode::Dataspace_capability(),
			_resources.pd.cap(),  _resources.pd,
			_resources.ram.cap(), _resources.ram,
			_resources.cpu.cap(), _initial_thread,
			_env.rm(), _address_space, _child_ep.rpc_ep(), *this);


}


void Target_child::start(Restorer &restorer)
{
	if(verbose_debug) Genode::log("Target_child::\033[33m", __func__, "\033[0m(from_state=", &restorer,")");

	_restorer = &restorer;

	_child = new (_md_alloc) Genode::Child(
			Genode::Rom_session_client{_resources.rom.parent_cap()}.dataspace(),
			Genode::Dataspace_capability(),
			_resources.pd.cap(),  _resources.pd,
			_resources.ram.cap(), _resources.ram,
			_resources.cpu.cap(), _initial_thread,
			_env.rm(), _address_space, _child_ep.rpc_ep(), *this);


}


void Target_child::print(Genode::Output &output) const
{
	using Genode::Hex;

	Genode::print(output, "##########################\n");
	Genode::print(output, "###    Target_child    ###\n");
	Genode::print(output, "##########################\n");

	// RM sessions
	{
		Genode::print(output, "RM sessions:\n");
		if(!_rm_root)
			Genode::print(output, " <empty>\n");
		else
		{
			const Rm_session_info *rm_info = _rm_root->rms_infos().first();
			if(!rm_info) Genode::print(output, " <empty>\n");
			while(rm_info)
			{
				Genode::print(output, " ", *rm_info, "\n");
				const Region_map_info *region_map_info = rm_info->session.region_map_infos().first();
				if(!region_map_info) Genode::print(output, "  <empty>\n");
				while(region_map_info)
				{
					Genode::print(output, "  ", *region_map_info, "\n");
					const Attached_region_info *attached_info = region_map_info->region_map.attached_regions().first();
					if(!attached_info) Genode::print(output, "  <empty>\n");
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
	}
	// LOG sessions
	{
		Genode::print(output, "LOG sessions:\n");
		if(!_log_root)
			Genode::print(output, " <empty>\n");
		else
		{
			const Log_session_info *info = _log_root->session_infos().first();
			if(!info) Genode::print(output, " <empty>\n");
			while(info)
			{
				Genode::print(output, " ", *info, "\n");
				info = info->next();
			}

		}
	}
	// Timer sessions
	{
		Genode::print(output, "Timer sessions:\n");
		if(!_timer_root)
			Genode::print(output, " <empty>\n");
		else
		{
			const Timer_session_info *info = _timer_root->session_infos().first();
			if(!info) Genode::print(output, " <empty>\n");
			while(info)
			{
				Genode::print(output, " ", *info, "\n");
				info = info->next();
			}

		}
	}
	// Signal contexts
	{
		Genode::print(output, "Signal contexts:\n");
		const Signal_context_info *info = _resources.pd.signal_context_infos().first();
		if(!info) Genode::print(output, " <empty>\n");
		while(info)
		{
			Genode::print(output, " ", *info, "\n");
			info = info->next();
		}
	}
	// Signal sources
	{
		Genode::print(output, "Signal sources:\n");
		const Signal_source_info *info = _resources.pd.signal_source_infos().first();
		if(!info) Genode::print(output, " <empty>\n");
		while(info)
		{
			Genode::print(output, " ", *info, "\n");
			info = info->next();
		}
	}
	// Threads
	{
		Genode::print(output, "Threads:\n");
		const Thread_info *info = _resources.cpu.thread_infos().first();
		if(!info) Genode::print(output, " <empty>\n");
		while(info)
		{
			Genode::print(output, " ", *info, "\n");
			info = info->next();
		}
	}
	// Address space
	{
		Genode::print(output, "Address space:\n");
		const Region_map_component &region_map = _resources.pd.address_space_component();
		Genode::print(output, region_map.cap(),
				", fault_handler ", region_map.parent_state().fault_handler,
				", ds ", region_map.parent_state().ds_cap, "\n");
		const Attached_region_info *info = region_map.attached_regions().first();
		if(!info) Genode::print(output, " <empty>\n");
		while(info)
		{
			Genode::print(output, " ", *info, "\n");
			info = info->next();
		}
	}
	// Stack area
	{
		Genode::print(output, "Stack area:\n");
		const Region_map_component &region_map = _resources.pd.stack_area_component();
		Genode::print(output, region_map.cap(),
				", fault_handler ", region_map.parent_state().fault_handler,
				", ds ", region_map.parent_state().ds_cap, "\n");
		const Attached_region_info *info = region_map.attached_regions().first();
		if(!info) Genode::print(output, " <empty>\n");
		while(info)
		{
			Genode::print(output, " ", *info, "\n");
			info = info->next();
		}
	}
	// Linker area
	{
		Genode::print(output, "Linker area:\n");
		const Region_map_component &region_map = _resources.pd.linker_area_component();
		Genode::print(output, region_map.cap(),
				", fault_handler ", region_map.parent_state().fault_handler,
				", ds ", region_map.parent_state().ds_cap, "\n");
		const Attached_region_info *info = region_map.attached_regions().first();
		if(!info) Genode::print(output, " <empty>\n");
		while(info)
		{
			Genode::print(output, " ", *info, "\n");
			info = info->next();
		}
	}
	// RAM allocations
	{
		Genode::print(output, "RAM allocations:\n");
		const Ram_dataspace_info *rd_info = _resources.ram.ram_dataspace_infos().first();
		if(!rd_info) Genode::print(output, " <empty>\n");
		while(rd_info)
		{
			Genode::print(output, " ", *rd_info, "\n");
			if(rd_info->mrm_info)
			{
				const Designated_dataspace_info *dd_info = rd_info->mrm_info->dd_infos.first();
				if(!dd_info) Genode::print(output, "  <empty>\n");
				while(dd_info)
				{
					Genode::print(output, "  ", *dd_info, "\n");
					dd_info = dd_info->next();
				}
			}
			rd_info = rd_info->next();
		}
	}

}


Genode::Service *Target_child::resolve_session_request(const char *service_name, const char *args)
{
	if(verbose_debug) Genode::log("Target_child::\033[33m", __func__, "\033[0m(", service_name, " ", args, ")");

	// Restoration hook
	if(!Genode::strcmp(service_name, "LOG") && _restorer)
	{
		_restorer->restore();

		_restorer = nullptr;
	}

	// TODO Support grandchildren: PD, CPU, and RAM session has also to be provided to them

	Genode::Service *service = 0;

	// Service is implemented locally?
	service = _local_services.find(service_name);
	if(service)
		return service;

	// Service known from parent?
	service = _parent_services.find(service_name);
	if(service)
		return service;

	// Service known from child?
	service = _child_services.find(service_name);
	if(service)
		return service;

	// Intercept these sessions
	if(!Genode::strcmp(service_name, "RM"))
	{
		_rm_root = new (_md_alloc) Rm_root(_env, _md_alloc, _resources_ep);
		service = new (_md_alloc) Genode::Local_service(service_name, _rm_root);
		_local_services.insert(service);
		if(verbose_debug) Genode::log("  inserted service into local_services");
	}
	else if(!Genode::strcmp(service_name, "LOG"))
	{
		_log_root = new (_md_alloc) Log_root(_env, _md_alloc, _resources_ep);
		service = new (_md_alloc) Genode::Local_service(service_name, _log_root);
		_local_services.insert(service);
		if(verbose_debug) Genode::log("  inserted service into local_services");
	}
	else if(!Genode::strcmp(service_name, "Timer"))
	{
		_timer_root = new (_md_alloc) Timer_root(_env, _md_alloc, _resources_ep);
		service = new (_md_alloc) Genode::Local_service(service_name, _timer_root);
		_local_services.insert(service);
		if(verbose_debug) Genode::log("  inserted service into local_services");
	}

	// Service not known, cannot intercept it
	if(!service)
	{
		service = new (_md_alloc) Genode::Parent_service(service_name);
		_parent_services.insert(service);
		Genode::warning("Unknown service: ", service_name);
	}

	return service;
}


void Target_child::filter_session_args(const char *service, char *args, Genode::size_t args_len)
{
	Genode::Arg_string::set_arg_string(args, args_len, "label", _name.string());
}
