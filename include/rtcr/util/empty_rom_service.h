#include <base/rpc_server.h>
#include <rom_session/connection.h>
#include <base/service.h>

namespace Rtcr {
	class Empty_ROM_session_component;
	class Empty_ROM_session_factory;
}

class Rtcr::Empty_ROM_session_component : public Genode::Rpc_object<Genode::Rom_session>
{
	Genode::Rpc_entrypoint &_ep;
	Genode::Env &_env;

	public:

	Empty_ROM_session_component(Genode::Rpc_entrypoint &ep, Genode::Env &env)
	:
	_ep(ep),
	_env(env)
	{
		_ep.manage(this);
	}

	~Empty_ROM_session_component()
	{
		_ep.dissolve(this);
	}

	Genode::Rom_dataspace_capability dataspace()
	{
		return Genode::Rom_dataspace_capability();
	}

	void sigh(Genode::Signal_context_capability /*sigh*/)
	{

	}
};

class Rtcr::Empty_ROM_session_factory : public Genode::Local_service<Rtcr::Empty_ROM_session_component>::Factory
{
	Genode::Allocator &_alloc;
	Genode::Rpc_entrypoint &_ep;
	Genode::Env &_env;

	public:

	Empty_ROM_session_factory(Genode::Allocator &alloc, Genode::Rpc_entrypoint &ep, Genode::Env &env)
	:
	_alloc(alloc),
	_ep(ep),
	_env(env)
	{ }

	~Empty_ROM_session_factory() { }

	Rtcr::Empty_ROM_session_component &create(Args const &, Genode::Affinity)
	{
		return *new (_alloc) Rtcr::Empty_ROM_session_component(_ep, _env);
	}

	void upgrade(Rtcr::Empty_ROM_session_component &, Args const &) { }

	void destroy(Rtcr::Empty_ROM_session_component &session)
	{
		Genode::destroy(_alloc, &session);
	}
};
