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
		Genode::size_t granularity, bool &bootstrap_phase, Genode::Session::Resources         resources)
:
	_env(env), _md_alloc(md_alloc), _resource_ep(ep), _bootstrap_phase(bootstrap_phase), _resources(resources)
{
	Genode::log("Custom services");
	/*pd_root  = new (_md_alloc) Rtcr::Pd_root(_env, _md_alloc, _resource_ep, _bootstrap_phase);
	pd_session = new (_md_alloc) Rtcr::Pd_session_component(_env,_md_alloc,_resource_ep,"sheep_counter","PD",foo,_resources,diag);
	pd_factory = new (_md_alloc) Genode::Local_service<Rtcr::Pd_session_component>::Single_session_factory(*pd_session);
	pd_service = new (_md_alloc) Genode::Local_service<Rtcr::Pd_session_component>(*pd_factory);

	cpu_root = new (_md_alloc) Rtcr::Cpu_root(_env, _md_alloc, _resource_ep, *pd_root, _bootstrap_phase);
	cpu_session = new (_md_alloc) Rtcr::Cpu_session_component(_env,_md_alloc,_resource_ep,*pd_root,"sheep_counter","CPU",foo);
	cpu_factory = new (_md_alloc) Genode::Local_service<Rtcr::Cpu_session_component>::Single_session_factory(*cpu_session);
	cpu_service = new (_md_alloc) Genode::Local_service<Rtcr::Cpu_session_component>(*cpu_factory);

	Genode::log("granularity ",granularity);

	ram_root = new (_md_alloc) Ram_root(_env, _md_alloc, _resource_ep, granularity, _bootstrap_phase);
	ram_session = new (_md_alloc) Pd_session_component(env,md_alloc,ep,"RAM","RAM",foo);
	ram_factory = new (_md_alloc) Genode::Local_service<Rtcr::Pd_session_component>::Single_session_factory(*ram_session);
	ram_service = new (_md_alloc) Genode::Local_service<Rtcr::Pd_session_component>(*ram_factory);*/
}

Target_child::Custom_services::~Custom_services()
{
	if(timer_root)    Genode::destroy(_md_alloc, timer_root);
	if(timer_service) Genode::destroy(_md_alloc, timer_service);

	if(log_root)    Genode::destroy(_md_alloc, log_root);
	if(log_service) Genode::destroy(_md_alloc, log_service);

	if(rm_root)    Genode::destroy(_md_alloc, rm_root);
	if(rm_service) Genode::destroy(_md_alloc, rm_service);

	if(rom_root)    Genode::destroy(_md_alloc, rom_root);
	if(rom_service) Genode::destroy(_md_alloc, rom_service);

	//if(ram_root)    Genode::destroy(_md_alloc, ram_root);
	//if(ram_service) Genode::destroy(_md_alloc, ram_service);

	//if(cpu_root)    Genode::destroy(_md_alloc, cpu_root);
	if(_cpu_service) Genode::destroy(_md_alloc, _cpu_service);

	//if(pd_root)    Genode::destroy(_md_alloc, pd_root);
	if(_pd_service) Genode::destroy(_md_alloc, _pd_service);
}


Genode::Service *Target_child::Custom_services::find(const char *service_name)
{
	Genode::Service *service = nullptr;

	if(!Genode::strcmp(service_name, "PD"))
	{
		service = _pd_service;
	}
	else if(!Genode::strcmp(service_name, "CPU"))
	{
		service = _cpu_service;
	}
	/*else if(!Genode::strcmp(service_name, "RAM"))
	{
		service = ram_service;
	}*/
	else if(!Genode::strcmp(service_name, "ROM"))
	{
		bool foo=false;
		if(!rom_root)    rom_root = new (_md_alloc) Rom_root(_env, _md_alloc, _resource_ep, _bootstrap_phase);
		if(!rom_session) rom_session = new (_md_alloc) Rom_session_component(_env,_md_alloc,_resource_ep,"sheep_counter","ROM",foo);
		if(!rom_factory) rom_factory = new (_md_alloc) Genode::Local_service<Rtcr::Rom_session_component>::Single_session_factory(*rom_session);
		if(!rom_service) rom_service = new (_md_alloc) Genode::Local_service<Rtcr::Rom_session_component>(*rom_factory);
		service = rom_service;
	}
	else if(!Genode::strcmp(service_name, "RM"))
	{
		bool foo=false;
		if(!rm_root)    rm_root = new (_md_alloc) Rm_root(_env, _md_alloc, _resource_ep, _bootstrap_phase);
		if(!rm_session)	rm_session = new (_md_alloc) Rm_session_component(_env,_md_alloc,_resource_ep,"RM",foo);
		if(!rm_factory)	rm_factory = new (_md_alloc) Genode::Local_service<Rtcr::Rm_session_component>::Single_session_factory(*rm_session);
		if(!rm_service) rm_service = new (_md_alloc) Genode::Local_service<Rtcr::Rm_session_component>(*rm_factory);
		service = rm_service;
	}
	else if(!Genode::strcmp(service_name, "LOG"))
	{
		bool foo=false;
		if(!log_root)    log_root = new (_md_alloc) Log_root(_env, _md_alloc, _resource_ep, _bootstrap_phase);
		if(!log_session) log_session = new (_md_alloc) Log_session_component(_env,_md_alloc,_resource_ep,"sheep_counter","LOG",foo);
		if(!log_factory) log_factory = new (_md_alloc) Genode::Local_service<Rtcr::Log_session_component>::Single_session_factory(*log_session);
		if(!log_service) log_service = new (_md_alloc) Genode::Local_service<Rtcr::Log_session_component>(*log_factory);
		service = log_service;
	}
	else if(!Genode::strcmp(service_name, "Timer"))
	{
		bool foo=false;
		if(!timer_root)    timer_root = new (_md_alloc) Timer_root(_env, _md_alloc, _resource_ep, _bootstrap_phase);
		if(!timer_session) timer_session = new (_md_alloc) Timer_session_component(_env,_md_alloc,_resource_ep,"TIMER",foo);
		if(!timer_factory) timer_factory = new (_md_alloc) Genode::Local_service<Rtcr::Timer_session_component>::Single_session_factory(*timer_session);
		if(!timer_service) timer_service = new (_md_alloc) Genode::Local_service<Rtcr::Timer_session_component>(*timer_factory);
		service = timer_service;
	}

	return service;
}



Target_child::Resources::Resources(Genode::Env &env, Genode::Allocator &md_alloc, const char *label, Custom_services &custom_services)
:
	//pd  (init_pd(label, *custom_services.pd_root, md_alloc)),
	//cpu (init_cpu(label, *custom_services.cpu_root, md_alloc)),
	//ram (init_ram(label, *custom_services.ram_root)),
	rom (env, label)
{
	// Donate ram quota to child
	// TODO Replace static quota donation with the amount of quota, the child needs
	//Genode::Ram_quota quota;
	//quota.value=10000000;
	//Genode::Cap_quota caps;
	//caps.value=100;
	//pd.ref_account(env.ram_session_cap());
	// Note: transfer goes directly to parent's ram session
	//env.pd().transfer_quota(pd.parent_cap(), quota);
	//env.pd().transfer_quota(pd.parent_cap(), caps);
	
	//Genode::log("donation complete");
}


Target_child::Resources::~Resources()
{ }


/*Pd_session_component &Target_child::Resources::init_pd(const char *label, Rtcr::Pd_root &pd_root, Genode::Allocator &)
{
	Genode::log("init pd");
	// Preparing argument string
	char args_buf[160];
	Genode::snprintf(args_buf, sizeof(args_buf), "virt_space=%d, ram_quota=%d, cap_quota=%d, label=\"%s\"", 1, 1024*1024, 100, label);
	Genode::log("init_pd ",(const char*)args_buf);
	// Issuing session method of pd_root
	//Rtcr::Pd_session_component* pd_session = 
	pd_root._create_session(args_buf);

	// Find created RPC object in pd_root's list
	Pd_session_component *pd_session = pd_root.session_infos().first();
	Genode::log("ram quota ",pd_session->ram_quota().value);
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
			Genode::Cpu_session::DEFAULT_PRIORITY, 1024*1024, 100, label);
	Genode::log("init_cpu ",(const char*)args_buf);
	// Issuing session method of Cpu_root
	Cpu_session_component *cpu_session = cpu_root._create_session(args_buf);

	// Find created RPC object in Cpu_root's list
	//Cpu_session_component *cpu_session = cpu_root.session_infos().first();
	//if(cpu_session) cpu_session = cpu_session->find_by_badge(cpu_cap.local_name());
	if(!cpu_session)
	{
		Genode::error("Creating custom PD session failed: Could not find PD session in PD root");
		throw Genode::Exception();
	}

	return *cpu_session;
}*/


/*Ram_session_component &Target_child::Resources::init_ram(const char *label, Ram_root &ram_root)
{
	// Preparing argument string
	char args_buf[160];
	Genode::snprintf(args_buf, sizeof(args_buf),
			"ram_quota=%lu, phys_start=0x%lx, phys_size=0x%lx, label=\"%s\"",
			4*1024*sizeof(long), 0UL, 0UL, label);

	// Issuing session method of Ram_root
	Genode::Session_capability ram_cap = ram_root.session(args_buf, Genode::Affinity());

	// Find created RPC object in Ram_root's list
	Ram_session_component *ram_session = ram_root.session_infos().first();
	if(ram_session) ram_session = ram_session->find_by_badge(ram_cap.local_name());
	if(!ram_session)
	{
		Genode::error("Creating custom PD session failed: Could not find PD session in PD root");
		throw Genode::Exception();
	}

	return *ram_session;
}*/


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
	/*using Genode::print;

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
	 RAM session
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
	}
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
	}*/

}

void Target_child::init(Genode::Pd_session &session, Genode::Capability<Genode::Pd_session> cap) 
{	
	session.ref_account(_env.pd_session_cap());
	//Genode::log("env ram_quota ",_env.pd().ram_quota().value);
	Genode::Ram_quota const ram_quota { 1000000 };
	//Genode::log("env cap_quota ",_env.pd().cap_quota().value);
	Genode::Cap_quota const cap_quota { 100 };
	//Genode::log("session ram_quota ",session.ram_quota().value);
	//Genode::log("session cap_quota ",session.cap_quota().value);
	try { _env.pd().transfer_quota(cap, ram_quota); }
	catch (Genode::Out_of_ram) {
		error(name(), ": unable to initialize RAM quota of PD"); }
	//Genode::log("init pd session trans cap");
	try { _env.pd().transfer_quota(cap, cap_quota); }
	catch (Genode::Out_of_caps) {
		error(name(), ": unable to initialize cap quota of PD"); }

	//Genode::log("RAM ",_task._desc.quota.value," caps ",_task._desc.caps);
}

Genode::Child_policy::Route Target_child::resolve_session_request(Genode::Service::Name const &name,
		                              Genode::Session_label const &label)
{
	
	Genode::log("Resolve session request ",name," ",label);
	//return Route { find_service(_parent_services,name), label, Genode::Session::Diag{false}};
	return Route { *_custom_services.find(name.string()), label, Genode::Session::Diag{false} };
	Genode::log("Could not find ",name);
	Genode::Child_policy::Route *foo=0;
	return *foo;
}

/*Genode::Child_policy::Route Target_child::resolve_session_request(Genode::Service::Name &service_name, Genode::Session_label &label)
{
	if(verbose_debug) Genode::log("Target_child::\033[33m", __func__, "\033[0m(", service_name, " ", label, ")");

	Genode::Child_policy::Route *service = 0;

	// Service known from parent?
	//service = _parent_services.find(service_name);
	if(service)
		return *service;

	// Service is a local, custom service?
	//service = _custom_services.find(service_name);
	if(service)
		return *service;

	// Service not known, cannot intercept it
	if(!service)
	{
		//service = new (_md_alloc) Genode::Parent_service(service_name);
		//_parent_services.insert(service);
		Genode::warning("Unknown service: ", service_name);
	}

	return *service;
}*/


/*void Target_child::filter_session_args(Genode::Service::Name const &service,
	                                 char * args, Genode::size_t args_len)
{
	Genode::log("session args ", service);
	Genode::Arg_string::set_arg_string(args, args_len, "label", _name.string());
}*/
