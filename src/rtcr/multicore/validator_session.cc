
#include "validator_session.h"
#include <base/log.h>

using namespace Rtcr;


Validator_session_component::Validator_session_component(Genode::Env &env, Genode::Allocator &md_alloc, const char *creation_args)
:
		_env(env)						,
		_md_alloc(md_alloc)				,
		_creation_args(creation_args)

{
	Genode::log("Validator session created \033[33m", __func__, "\033[0m");
}


Validator_session_component::~Validator_session_component()

{}


bool Validator_session_component::dataspace_available()
{
	Genode::log("Validator session function \033[33m", __func__, "\033[0m called");
	return true;
}


Genode::Dataspace_capability Validator_session_component::get_dataspace()
{
	Genode::log("Validator session function \033[33m", __func__, "\033[0m called");
	return nullptr;
}






Validator_root::Validator_root(Genode::Env &env, Genode::Allocator &md_alloc, Genode::Entrypoint &session_ep)
:
		_env(env),
		_md_alloc(md_alloc),
		_session_ep(session_ep)

{}



Validator_root::~Validator_root()

{}



Validator_session_component *Validator_root::_create_session(const char *args)
{
	Validator_session_component *session =
				new (md_alloc()) Validator_session_component(_env, _md_alloc, args);

	Genode::Lock::Guard lock(_objs_lock);
	_session_rpc_objs.insert(session);

	return session;
}



void Validator_root::_upgrade_session(Validator_session_component *session, const char *upgrade_args)
{

}



void Validator_root::_destroy_session(Validator_session_component *session)
{
	Genode::destroy(_md_alloc, session);
}
