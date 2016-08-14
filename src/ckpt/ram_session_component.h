/*
 * \brief Intercepting Ram session
 * \author Denis Huber
 * \date 2016-08-12
 */

#ifndef _RTCR_RAM_SESSION_COMPONENT_H_
#define _RTCR_RAM_SESSION_COMPONENT_H_

/* Genode includes */
#include <base/log.h>
#include <base/rpc_server.h>
#include <ram_session/connection.h>

namespace Rtcr {
	class Ram_session_component;
	using namespace Genode;
}

class Rtcr::Ram_session_component : public Rpc_object<Ram_session>
{
private:
	static constexpr bool verbose = true;

	Entrypoint &_ep;
	Allocator  &_md_alloc;

	/**
	 * Connection to the parent Ram session (usually core's Ram session)
	 */
	Ram_connection _parent_ram;

public:

	Ram_session_component(Entrypoint &ep, Allocator &md_alloc)
	:
		_ep(ep), _md_alloc(md_alloc), _parent_ram()
	{
		_ep.manage(*this);
		if(verbose)
		{
			log("Ram_session_component created");
			//log("Arguments: env=", &env, ", md_alloc=", &md_alloc);
			//log("State: _env=", &_env, ", _md_alloc=", &_md_alloc, ", _parent_ram=", _parent_ram.local_name());
			//log("n=", _parent_ram.service_name(), " ln=", _parent_ram.local_name(), " v=", _parent_ram.valid()?"true":"false");
			//log("q=", _parent_ram.quota(), " u=", _parent_ram.used(), " a=", _parent_ram.avail());
		}
	}

	~Ram_session_component()
	{
		_ep.dissolve(*this);
		if(verbose) log("Ram_session_component destroyed");
	}

	Ram_dataspace_capability alloc(size_t size, Cache_attribute cached) override
	{
		if(verbose) log("alloc()");
		return _parent_ram.alloc(size, cached);
	}

	void free(Ram_dataspace_capability ds) override
	{
		if(verbose) log("free()");
		_parent_ram.free(ds);
	}

	int ref_account(Ram_session_capability ram_session) override
	{
		if(verbose) log("ref_account()");
		return _parent_ram.ref_account(ram_session);
	}

	int transfer_quota(Ram_session_capability ram_session, Genode::size_t amount) override
	{
		if(verbose) log("transfer_quota()");
		return _parent_ram.transfer_quota(ram_session, amount);
	}

	Genode::size_t quota() override
	{
		if(verbose) log("quota()");
		return _parent_ram.quota();
	}

	Genode::size_t used() override
	{
		if(verbose) log("used()");
		return _parent_ram.used();
	}

};

#endif /* _RTCR_RAM_SESSION_COMPONENT_H_ */
