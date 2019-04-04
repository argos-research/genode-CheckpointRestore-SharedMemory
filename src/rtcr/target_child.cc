/*
 * \brief  Child creation
 * \author Denis Huber
 * \date   2016-08-04
 */

#include "rtcr/target_child.h"
#include "rtcr/restorer.h"

namespace Fiasco {
#include <l4/sys/kdebug.h>
}

using namespace Rtcr;

Target_child::Custom_services::Custom_services(Genode::Env &env, Genode::Allocator &md_alloc, Genode::Entrypoint &ep,
		Genode::size_t, bool &bootstrap_phase, Genode::Session::Resources         resources)
:
	_env(env), _md_alloc(md_alloc), _resource_ep(ep), _bootstrap_phase(bootstrap_phase), _resources(resources)
{
	Genode::log("Custom services");
	pd_root  = new (_md_alloc) Rtcr::Pd_root(_env, _md_alloc, _resource_ep, _bootstrap_phase);

	cpu_root = new (_md_alloc) Rtcr::Cpu_root(_env, _md_alloc, _resource_ep, *pd_root, _bootstrap_phase);

	rom_root = new (_md_alloc) Rom_root(_env, _md_alloc, _resource_ep, _bootstrap_phase);

	rm_root = new (_md_alloc) Rm_root(_env, _md_alloc, _resource_ep, _bootstrap_phase);

	log_root = new (_md_alloc) Log_root(_env, _md_alloc, _resource_ep, _bootstrap_phase);

	timer_root = new (_md_alloc) Timer_root(_env, _md_alloc, _resource_ep, _bootstrap_phase);

}

Target_child::Custom_services::~Custom_services()
{
	if(timer_service) Genode::destroy(_md_alloc, timer_service);

	if(log_service) Genode::destroy(_md_alloc, log_service);

	if(rm_service) Genode::destroy(_md_alloc, rm_service);

	if(rom_service) Genode::destroy(_md_alloc, rom_service);

	if(cpu_service) Genode::destroy(_md_alloc, cpu_service);

	if(pd_service) Genode::destroy(_md_alloc, pd_service);
}


Genode::Service *Target_child::Custom_services::find(const char *service_name)
{
	Genode::Service *service = nullptr;

	if(!Genode::strcmp(service_name, "PD"))
	{
		service = pd_service;
	}
	else if(!Genode::strcmp(service_name, "CPU"))
	{
		service = cpu_service;
	}
	else if(!Genode::strcmp(service_name, "ROM"))
	{
		service = rom_service;
	}
	else if(!Genode::strcmp(service_name, "RM"))
	{
		service = rm_service;
	}
	else if(!Genode::strcmp(service_name, "LOG"))
	{
		service = log_service;
	}
	else if(!Genode::strcmp(service_name, "Timer"))
	{
		service = timer_service;
	}

	return service;
}



Target_child::Resources::Resources(Genode::Env &, Genode::Allocator &md_alloc, const char *label, Custom_services &custom_services)
:
	pd  (init_pd(label, *custom_services.pd_root, md_alloc)),
	cpu (init_cpu(label, *custom_services.cpu_root, md_alloc)),
	rom (init_rom(label, *custom_services.rom_root, md_alloc)),
	rm (init_rm(label, *custom_services.rm_root, md_alloc)),
	log (init_log(label, *custom_services.log_root, md_alloc)),
	timer (init_timer(label, *custom_services.timer_root, md_alloc))
{
	
}


Target_child::Resources::~Resources()
{ }


Pd_session_component &Target_child::Resources::init_pd(const char *label, Rtcr::Pd_root &pd_root, Genode::Allocator &)
{
	Genode::log("init pd");
	// Preparing argument string
	char args_buf[160];
	Genode::snprintf(args_buf, sizeof(args_buf), "virt_space=%d, ram_quota=%d, cap_quota=%d, label=\"%s\"", 1, 1024*1024, 10, label);
	Genode::log("init_pd ",(const char*)args_buf);
	// Issuing session method of pd_root
	Rtcr::Pd_session_component* pd_session = 
	pd_root._create_session(args_buf);

	// Find created RPC object in pd_root's list
	//Pd_session_component *pd_session = pd_root.session_infos().first();
	//Genode::log("ram quota ",pd_session->ram_quota().value);
	//if(pd_session) pd_session = pd_session->find_by_badge(pd_cap.local_name());
	if(!pd_session)
	{
		Genode::error("Creating custom PD session failed: Could not find PD session in PD root");
		throw Genode::Exception();
	}
	return *pd_session;
}


Cpu_session_component &Target_child::Resources::init_cpu(const char *label, Cpu_root &cpu_root, Genode::Allocator &)
{
	// Preparing argument string
	char args_buf[160];
	Genode::snprintf(args_buf, sizeof(args_buf),
			"priority=0x%x, ram_quota=%u, cap_quota=%d, label=\"%s\"",
			Genode::Cpu_session::DEFAULT_PRIORITY, 1024*1024, 10, label);
	Genode::log("init_cpu ",(const char*)args_buf);
	// Issuing session method of Cpu_root
	Cpu_session_component *cpu_session = cpu_root._create_session(args_buf);

	// Find created RPC object in Cpu_root's list
	//Cpu_session_component *cpu_session = cpu_root.session_infos().first();
	//if(cpu_session) cpu_session = cpu_session->find_by_badge(cpu_cap.local_name());
	if(!cpu_session)
	{
		Genode::error("Creating custom CPU session failed: Could not find CPU session in CPU root");
		throw Genode::Exception();
	}

	return *cpu_session;
}

Rom_session_component &Target_child::Resources::init_rom(const char *label, Rom_root &rom_root, Genode::Allocator &)
{
	// Preparing argument string
	char args_buf[160];
	Genode::snprintf(args_buf, sizeof(args_buf),
			"ram_quota=%u, cap_quota=%d, label=\"%s\"",
			 1024*1024, 10, label);
	Genode::log("init_rom ",(const char*)args_buf);
	// Issuing session method of Rom_root
	Rom_session_component *rom_session = rom_root._create_session(args_buf);

	if(!rom_session)
	{
		Genode::error("Creating custom ROM session failed: Could not find ROM session in ROM root");
		throw Genode::Exception();
	}

	return *rom_session;
}

Rm_session_component &Target_child::Resources::init_rm(const char *label, Rm_root &rm_root, Genode::Allocator &)
{
	// Preparing argument string
	char args_buf[160];
	Genode::snprintf(args_buf, sizeof(args_buf),
			"ram_quota=%u, cap_quota=%d, label=\"%s\"",
			 1024*1024, 10, label);
	Genode::log("init_rm ",(const char*)args_buf);
	// Issuing session method of Rm_root
	Rm_session_component *rm_session = rm_root._create_session(args_buf);

	if(!rm_session)
	{
		Genode::error("Creating custom RM session failed: Could not find RM session in RM root");
		throw Genode::Exception();
	}

	return *rm_session;
}

Log_session_component &Target_child::Resources::init_log(const char *label, Log_root &log_root, Genode::Allocator &)
{
	// Preparing argument string
	char args_buf[160];
	Genode::snprintf(args_buf, sizeof(args_buf),
			"ram_quota=%u, cap_quota=%d, label=\"%s\"",
			 1024*1024, 10, label);
	Genode::log("init_log ",(const char*)args_buf);
	// Issuing session method of Rm_root
	Log_session_component *log_session =log_root._create_session(args_buf);

	if(!log_session)
	{
		Genode::error("Creating custom LOG session failed: Could not find LOG session in LOG root");
		throw Genode::Exception();
	}

	return *log_session;
}

Timer_session_component &Target_child::Resources::init_timer(const char *label, Timer_root &timer_root, Genode::Allocator &)
{
	// Preparing argument string
	char args_buf[160];
	Genode::snprintf(args_buf, sizeof(args_buf),
			"ram_quota=%u, cap_quota=%d, label=\"%s\"",
			 1024*1024, 10, label);
	Genode::log("init_timer ",(const char*)args_buf);
	// Issuing session method of Rm_root
	Timer_session_component *timer_session = timer_root._create_session(args_buf);

	if(!timer_session)
	{
		Genode::error("Creating custom TIMER session failed: Could not find TIMER session in TIMER root");
		throw Genode::Exception();
	}

	return *timer_session;
}

void Target_child::init(Genode::Pd_session &session, Genode::Capability<Genode::Pd_session> /*cap*/) { 
	Genode::log("init ram");	
	Genode::Ram_quota quota;
	quota.value=1000000;
	Genode::Cap_quota caps;
	caps.value=100;
	session.ref_account(_env.pd_session_cap());
	try { _env.pd().transfer_quota(_resources.pd.parent_cap(), caps); }
	catch (Genode::Out_of_caps) { }	
	try { _env.pd().transfer_quota(_resources.pd.parent_cap(), quota); }
	catch (Genode::Out_of_ram) { }
	Genode::log("ram after transfer: ",session.ram_quota().value);
	Genode::log("cap after transfer: ",session.cap_quota().value);

}

void Target_child::init(Genode::Cpu_session &, Genode::Capability<Genode::Cpu_session>) {
	Genode::log("init cpu");
}

Target_child::Target_child(Genode::Env &env, Genode::Allocator &md_alloc,
		Genode::Registry<Genode::Registered<Genode::Parent_service> > &parent_services, const char *name, Genode::size_t granularity)
:
	_in_bootstrap    (true),
	_name            (name),
	_env             (env),
	_md_alloc        (md_alloc),
	_resources_ep    (_env, 16*1024, "resources ep"),
	_child_ep        (_env, 16*1024, "child ep"),
	_granularity     (granularity),
	_restorer        (nullptr),
	_custom_services (_env, _md_alloc, _resources_ep, _granularity, _in_bootstrap,Genode::session_resources_from_args("cap_quota=100,ram_quota=1000000")),
	_resources       (_env, _md_alloc, _name.string(), _custom_services),
	_address_space   (_resources.pd.address_space()),
	_parent_services (parent_services),
	_child           (nullptr)
{
	bool bar=false;
	_custom_services.pd_session = &_resources.pd;
	_custom_services.pd_factory = new (_md_alloc) Rtcr::Local_pd_factory(_env, _md_alloc, _resources_ep, name, name, bar, Genode::session_resources_from_args("cap_quota=10,ram_quota=1000000"), Genode::Session::Diag(),_resources.pd);
	_custom_services.pd_service = new (_md_alloc) Genode::Local_service<Rtcr::Pd_session_component>(*_custom_services.pd_factory);
	_custom_services.cpu_session = &_resources.cpu;
	_custom_services.cpu_factory = new (_md_alloc) Rtcr::Local_cpu_factory(_env, _md_alloc, _resources_ep, _custom_services.pd_root, name, name, bar, _resources.cpu);
	_custom_services.cpu_service = new (_md_alloc) Genode::Local_service<Rtcr::Cpu_session_component>(*_custom_services.cpu_factory);
	_custom_services.rom_factory = new (_md_alloc) Rtcr::Local_rom_factory(_env, _md_alloc, _resources_ep, bar, _resources.rom);
	_custom_services.rom_service = new (_md_alloc) Genode::Local_service<Rtcr::Rom_session_component>(*_custom_services.rom_factory);
	_custom_services.rm_factory = new (_md_alloc) Rtcr::Local_rm_factory(_env, _md_alloc, _resources_ep, bar, _resources.rm);
	_custom_services.rm_service = new (_md_alloc) Genode::Local_service<Rtcr::Rm_session_component>(*_custom_services.rm_factory);
	_custom_services.log_factory = new (_md_alloc) Rtcr::Local_log_factory(_env, _md_alloc, _resources_ep, bar, _resources.log);
	_custom_services.log_service = new (_md_alloc) Genode::Local_service<Rtcr::Log_session_component>(*_custom_services.log_factory);
	_custom_services.timer_factory = new (_md_alloc) Rtcr::Local_timer_factory(_env, _md_alloc, _resources_ep, bar, _resources.timer);
	_custom_services.timer_service = new (_md_alloc) Genode::Local_service<Rtcr::Timer_session_component>(*_custom_services.timer_factory);
	if(verbose_debug) Genode::log("\033[33m", __func__, "\033[0m(child=", _name.string(), ")");
	_in_bootstrap = false;
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

	_child = new (_md_alloc) Genode::Child (
			/*_resources.rom.dataspace(),
			Genode::Dataspace_capability(),
			_resources.pd.cap(),  _resources.pd,
			_resources.ram.cap(), _resources.ram,
			_resources.cpu.cap(), _initial_thread,*/
			_env.rm(), /*_address_space,*/ _child_ep.rpc_ep(), *this/*,
			*_custom_services.pd_service,
			*_custom_services.ram_service,
			*_custom_services.cpu_service*/);
	Genode::log("done start");


}


void Target_child::start(Restorer &restorer)
{
	if(verbose_debug) Genode::log("Target_child::\033[33m", __func__, "\033[0m(from_restorer=", &restorer,")");

	_child = new (_md_alloc) Genode::Child (
			/*Genode::Dataspace_capability(),
			Genode::Dataspace_capability(),
			_resources.pd.cap(),  _resources.pd,
			_resources.ram.cap(), _resources.ram,
			_resources.cpu.cap(), _initial_thread,*/
			_env.rm(), /*_address_space,*/ _child_ep.rpc_ep(), *this/*,
			*_custom_services.pd_service,
			*_custom_services.ram_service,
			*_custom_services.cpu_service*/);

	//enter_kdebug("before restore");

	restorer.restore();
}


void Target_child::pause()
{
	if(verbose_debug) Genode::log("Target_child::\033[33m", __func__, "\033[0m()");

	// Pause all threads of all sessions

	// Iterate through every session
	Cpu_session_component *cpu_session = _custom_services.cpu_root->session_infos().first();
	while(cpu_session)
	{
		// Iterate through every CPU thread
		Cpu_thread_component *cpu_thread = cpu_session->parent_state().cpu_threads.first();
		while(cpu_thread)
		{
			// Pause the CPU thread
			Genode::Cpu_thread_client client{cpu_thread->parent_cap()};
			client.pause();

			cpu_thread = cpu_thread->next();
		}

		cpu_session = cpu_session->next();
	}
}


void Target_child::resume()
{
	if(verbose_debug) Genode::log("Target_child::\033[33m", __func__, "\033[0m()");

	// Pause all threads of all sessions

	// Iterate through every session
	Cpu_session_component *cpu_session = _custom_services.cpu_root->session_infos().first();
	while(cpu_session)
	{
		// Iterate through every CPU thread
		Cpu_thread_component *cpu_thread = cpu_session->parent_state().cpu_threads.first();
		while(cpu_thread)
		{
			// Pause the CPU thread
			Genode::Cpu_thread_client client{cpu_thread->parent_cap()};
			client.resume();

			cpu_thread = cpu_thread->next();
		}

		cpu_session = cpu_session->next();
	}
}


void Target_child::print(Genode::Output &output) const
{
	using Genode::print;

	print(output, "##########################\n");
	print(output, "###    Target_child    ###\n");
	print(output, "##########################\n");

	// PD session
	{
		print(output, "PD sessions:\n");
		if(!_custom_services.pd_root)
			print(output, " <empty>\n");
		else
		{
			Pd_session_component const *pd_session = _custom_services.pd_root->session_infos().first();
			if(!pd_session) print(output, " <empty>\n");
			while(pd_session)
			{
				print(output, " ", pd_session->cap(), " ", pd_session->parent_state(), "\n");

				// Signal contexts
				print(output, " Signal contexts:\n");

				Signal_context_info const *context_info = pd_session->parent_state().signal_contexts.first();
				if(!context_info) print(output, "  <empty>\n");
				while(context_info)
				{
					print(output, "  ", *context_info, "\n");

					context_info = context_info->next();
				}

				// Signal sources
				print(output, " Signal sources:\n");

				Signal_source_info const *source_info = pd_session->parent_state().signal_sources.first();
				if(!source_info) print(output, "  <empty>\n");
				while(source_info)
				{
					print(output, "  ", *source_info, "\n");

					source_info = source_info->next();
				}

				// Native caps
				print(output, " Native caps:\n");

				Native_capability_info const *native_info = pd_session->parent_state().native_caps.first();
				if(!native_info) print(output, "  <empty>\n");
				while(native_info)
				{
					print(output, "  ", *native_info, "\n");

					native_info = native_info->next();
				}

				// Address space
				Region_map_component const &address_space = pd_session->address_space_component();
				print(output, " Address space: ", address_space.cap(), " ", address_space.parent_state(), "\n");

				Attached_region_info const *attached_info = address_space.parent_state().attached_regions.first();
				if(!attached_info) print(output, "  <empty>\n");
				while(attached_info)
				{
					print(output, "  ", *attached_info, "\n");

					attached_info = attached_info->next();
				}

				// Stack area
				Region_map_component const &stack_area = pd_session->stack_area_component();
				print(output, " Stack area: ", stack_area.cap(), " ", stack_area.parent_state(), "\n");

				attached_info = stack_area.parent_state().attached_regions.first();
				if(!attached_info) print(output, "  <empty>\n");
				while(attached_info)
				{
					print(output, "  ", *attached_info, "\n");

					attached_info = attached_info->next();
				}

				// Linker area
				Region_map_component const &linker_area = pd_session->linker_area_component();
				print(output, " Linker area: ", linker_area.cap(), " ", linker_area.parent_state(), "\n");

				attached_info = linker_area.parent_state().attached_regions.first();
				if(!attached_info) print(output, "  <empty>\n");
				while(attached_info)
				{
					print(output, "  ", *attached_info, "\n");

					attached_info = attached_info->next();
				}

				pd_session = pd_session->next();
			}
		}
	}
	// CPU session
	{
		print(output, "CPU sessions:\n");
		Cpu_session_component const *cpu_session = _custom_services.cpu_root->session_infos().first();
		if(!cpu_session) print(output, " <empty>\n");
		while(cpu_session)
		{
			print(output, " ", cpu_session->cap(), " ", cpu_session->parent_state(), "\n");

			Cpu_thread_component const *cpu_thread = cpu_session->parent_state().cpu_threads.first();
			if(!cpu_thread) print(output, "  <empty>\n");
			while(cpu_thread)
			{
				print(output, "  ", cpu_thread->cap(), " ", cpu_thread->parent_state(), "\n");

				cpu_thread = cpu_thread->next();
			}

			cpu_session = cpu_session->next();
		}
	}
	/* RAM session
	{
		print(output, "RAM sessions:\n");
		Ram_session_component const *ram_session = _custom_services.ram_root->session_infos().first();
		if(!ram_session) print(output, " <empty>\n");
		while(ram_session)
		{
			print(output, " ", ram_session->cap(), " ", ram_session->parent_state(), "\n");
			Ram_dataspace_info const *ramds_info = ram_session->parent_state().ram_dataspaces.first();
			if(!ramds_info) print(output, "  <empty>\n");
			while(ramds_info)
			{
				print(output, "  ", *ramds_info, "\n");

				if(ramds_info->mrm_info)
				{
					Designated_dataspace_info const *dd_info = ramds_info->mrm_info->dd_infos.first();
					if(!dd_info) print(output, "   <empty>\n");
					while(dd_info)
					{
						print(output, "   ", *dd_info, "\n");
						dd_info = dd_info->next();
					}
				}

				ramds_info = ramds_info->next();
			}

			ram_session = ram_session->next();
		}
	}*/
	// RM sessions
	{
		print(output, "RM sessions:\n");
		if(!_custom_services.rm_root)
			print(output, " <empty>\n");
		else
		{
			Rm_session_component const *rm_session = _custom_services.rm_root->session_infos().first();
			if(!rm_session) print(output, " <empty>\n");
			while(rm_session)
			{
				print(output, " ", rm_session->cap(), " ", rm_session->parent_state(), "\n");

				Region_map_component const *region_map = rm_session->parent_state().region_maps.first();
				if(!region_map) print(output, "  <empty>\n");
				while(region_map)
				{
					print(output, "  ", region_map->cap(), " ", region_map->parent_state(), "\n");

					Attached_region_info const *attached_info = region_map->parent_state().attached_regions.first();
					if(!attached_info) print(output, "  <empty>\n");
					while(attached_info)
					{
						print(output, "   ", *attached_info, "\n");

						attached_info = attached_info->next();
					}
					region_map = region_map->next();
				}
				rm_session = rm_session->next();
			}
		}
	}
	// LOG sessions
	{
		print(output, "LOG sessions:\n");
		if(!_custom_services.log_root)
			print(output, " <empty>\n");
		else
		{
			Log_session_component const *log_session = _custom_services.log_root->session_infos().first();
			if(!log_session) print(output, " <empty>\n");
			while(log_session)
			{
				print(output, " ", log_session->cap(), " ", log_session->parent_state(), "\n");
				log_session = log_session->next();
			}

		}
	}
	// Timer sessions
	{
		print(output, "Timer sessions:\n");
		if(!_custom_services.timer_root)
			print(output, " <empty>\n");
		else
		{
			Timer_session_component const *timer_session = _custom_services.timer_root->session_infos().first();
			if(!timer_session) print(output, " <empty>\n");
			while(timer_session)
			{
				print(output, " ", timer_session->cap(), " ", timer_session->parent_state(), "\n");
				timer_session = timer_session->next();
			}

		}
	}

}

Genode::Child_policy::Route Target_child::resolve_session_request(Genode::Service::Name const &name,
		                              Genode::Session_label const &label)
{
	
	Genode::log("Resolve session request ",name," ",label);
	/*if(name!="PD"&&name!="CPU")
	{
		Genode::log("parent service ",name);
		return Route { find_service(_parent_services,name), label, Genode::Session::Diag{false}};
	}*/
	Genode::log("local service ",name);
	return Route { *_custom_services.find(name.string()), label, Genode::Session::Diag{false} };
	Genode::log("Could not find ",name);
	Genode::Child_policy::Route *foo=0;
	return *foo;
}

Rtcr::Pd_session_component &Local_pd_factory::create(Args const &, Genode::Affinity)
{
	Genode::log("create custom pd session from factory");
	return _pd;
}

Rtcr::Cpu_session_component &Local_cpu_factory::create(Args const &, Genode::Affinity)
{
	Genode::log("create custom cpu session from factory");
	return _cpu;
}

Rtcr::Rom_session_component &Local_rom_factory::create(Args const &, Genode::Affinity)
{
	Genode::log("create custom rom session from factory");
	return _rom;
}

Rtcr::Rm_session_component &Local_rm_factory::create(Args const &, Genode::Affinity)
{
	Genode::log("create custom rom session from factory");
	return _rm;
}

Rtcr::Log_session_component &Local_log_factory::create(Args const &, Genode::Affinity)
{
	Genode::log("create custom rom session from factory");
	return _log;
}

Rtcr::Timer_session_component &Local_timer_factory::create(Args const &, Genode::Affinity)
{
	Genode::log("create custom rom session from factory");
	return _timer;
}

void Local_pd_factory::upgrade(Rtcr::Pd_session_component &, Args const &)
{

}

void Local_pd_factory::destroy(Rtcr::Pd_session_component &)
{

}

void Local_cpu_factory::upgrade(Rtcr::Cpu_session_component &, Args const &)
{

}

void Local_cpu_factory::destroy(Rtcr::Cpu_session_component &)
{

}

void Local_rom_factory::upgrade(Rtcr::Rom_session_component &, Args const &)
{

}

void Local_rom_factory::destroy(Rtcr::Rom_session_component &)
{

}

void Local_rm_factory::upgrade(Rtcr::Rm_session_component &, Args const &)
{

}

void Local_rm_factory::destroy(Rtcr::Rm_session_component &)
{

}

void Local_log_factory::upgrade(Rtcr::Log_session_component &, Args const &)
{

}

void Local_log_factory::destroy(Rtcr::Log_session_component &)
{

}

void Local_timer_factory::upgrade(Rtcr::Timer_session_component &, Args const &)
{

}

void Local_timer_factory::destroy(Rtcr::Timer_session_component &)
{

}
