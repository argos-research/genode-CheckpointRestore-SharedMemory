/*
 * \brief Intercepting Pd session
 * \author Denis Huber
 * \date 2016-08-03
 */

#ifndef _RTCR_PD_SESSION_COMPONENT_H_
#define _RTCR_PD_SESSION_COMPONENT_H_

/* Genode includes */
#include <base/log.h>
#include <base/rpc_server.h>
#include <pd_session/connection.h>

namespace Rtcr {
	class Pd_session_component;
	using namespace Genode;
}

class Rtcr::Pd_session_component : public Rpc_object<Pd_session>
{
private:
	static constexpr bool verbose = true;

	Env        &_env;
	Entrypoint &_ep;
	Allocator  &_md_alloc;
	/**
	 * Connection to parent's pd connection, usually from core
	 */
	Pd_connection _parent_pd;

public:
	/**
	 * Constructor
	 */
	Pd_session_component(Env &env, Entrypoint &ep, Allocator &md_alloc, const char *label)
	:
		_env(env),
		_ep(ep),
		_md_alloc(md_alloc),
		_parent_pd(env, label)
	{
		_ep.manage(*this);
		if(verbose)
		{
			log("Pd_session_component created");
			//log("Arguments: env=", &env, ", md_alloc=", &md_alloc, ", label=", label);
			//log("State: _env=", &_env, ", _md_alloc=", &_md_alloc, ", _parent_pd=", _parent_pd.local_name());
		}
	}

	~Pd_session_component()
	{
		_ep.dissolve(*this);
		if(verbose) log("Pd_session_component destroyed");
	}

	void print_cap()
	{
		//log("__Cap__");
		log("local_name: ", this->cap().local_name());
		/*log("cap_index id: ", (unsigned int)this->cap().idx()->id());
		log("valid: ", this->cap().valid() ? "true" : "false");

		log("__Rpc__");
		log("local_name: ", this->Rpc_object<Pd_session>::cap().local_name());
		log("cap_index id: ", (unsigned int)this->Rpc_object<Pd_session>::cap().idx()->id());
		log("valid: ", this->Rpc_object<Pd_session>::cap().valid() ? "true" : "false");*/
	}

	Pd_session_capability parent_pd_cap()
	{
		return _parent_pd.cap();
	}

	/**************************
	 ** Pd_session interface **
	 **************************/
	void assign_parent(Capability<Parent> parent) override
	{
		if(verbose) log("assign_parent()");
		_parent_pd.assign_parent(parent);
	}

	bool assign_pci(addr_t addr, uint16_t bdf) override
	{
		if(verbose) log("assign_pci()");
		return _parent_pd.assign_pci(addr, bdf);
	}

	Signal_source_capability alloc_signal_source() override
	{
		if(verbose) log("alloc_signal_source()");
		return _parent_pd.alloc_signal_source();
	}

	void free_signal_source(Signal_source_capability cap) override
	{
		if(verbose) log("free_signal_source()");
		_parent_pd.free_signal_source(cap);
	}

	Capability<Signal_context> alloc_context(Signal_source_capability source,
			unsigned long imprint) override
	{
		if(verbose) log("alloc_context()");
		return _parent_pd.alloc_context(source, imprint);
	}

	void free_context(Capability<Signal_context> cap) override
	{
		if(verbose) log("free_context()");
		_parent_pd.free_context(cap);
	}

	void submit(Capability<Signal_context> context, unsigned cnt) override
	{
		if(verbose) log("submit()");
		_parent_pd.submit(context, cnt);
	}

	Native_capability alloc_rpc_cap(Native_capability ep) override
	{
		if(verbose) log("alloc_rpc_cap()");
		return _parent_pd.alloc_rpc_cap(ep);
	}

	void free_rpc_cap(Native_capability cap) override
	{
		if(verbose) log("free_rpc_cap()");
		_parent_pd.free_rpc_cap(cap);
	}

	Capability<Region_map> address_space() override
	{
		if(verbose) log("address_space()");
		return _parent_pd.address_space();
	}

	Capability<Region_map> stack_area() override
	{
		if(verbose) log("stack_area()");
		return _parent_pd.stack_area();
	}

	Capability<Region_map> linker_area() override
	{
		if(verbose) log("linker_area()");
		return _parent_pd.linker_area();
	}

	Capability<Native_pd> native_pd() override
	{
		if(verbose) log("native_pd()");
		return _parent_pd.native_pd();
	}

};

#endif /* _RTCR_PD_SESSION_COMPONENT_H_ */
