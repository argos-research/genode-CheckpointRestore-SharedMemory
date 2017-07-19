
#include "validator_session.h"
#include <base/log.h>

using namespace Rtcr;


Validator_session_component::Validator_session_component(Genode::Env &env, Genode::Allocator &md_alloc, const char *creation_args, Target_state ts, Genode::size_t ds_size)
:
		_env(env)						,
		_md_alloc(md_alloc)				,
		_creation_args(creation_args)	,
		_ts(ts)							,
		_ds_size(ds_size)

{
	// Find first datspace in offline storage with size _ds_size
	_ds = _ts.get_ram_sessions().first()->stored_ramds_infos.first()->find_by_size(_ds_size);

	Genode::log("Validator session created \033[33m", __func__, "\033[0m");


}


Validator_session_component::~Validator_session_component()

{}


bool Validator_session_component::dataspace_available()
{
	Genode::log("Validator session function \033[33m", __func__, "\033[0m called");

	return _ds ? true : false;
}


Genode::Dataspace_capability Validator_session_component::get_dataspace()
{
//	Genode::log("---------------------------------------------------------------------");
	Genode::log("Validator session function \033[33m", __func__, "\033[0m called");
	if(_ds)
	{
		Genode::log("Current dataspace: ", _ds);
		Genode::Dataspace_capability ds_cap = _ds->memory_content;
		Genode::log("Stored capability: ", ds_cap);

		if (_ds->next())
			_ds = _ds->next()->find_by_size(_ds_size);
		else
			_ds = _ds->next();

		Genode::log("Next dataspace: ", _ds);
		Genode::log("Returning: ", ds_cap);
		return ds_cap;
	}
	else
	{
		return Genode::Dataspace_capability();
	}
}






Validator_root::Validator_root(Genode::Env &env, Genode::Allocator &md_alloc, Genode::Entrypoint &session_ep, Target_state &ts, Genode::size_t ds_size)
:
		Root_component<Validator_session_component>(session_ep, md_alloc),
		_env(env),
		_md_alloc(md_alloc),
		_ep(session_ep),
		_ts(ts),
		_ds_size(ds_size)

{
	Genode::log("\033[31mValidator_root created: ",__func__,"\033[0m");
}



Validator_root::~Validator_root()

{}



Validator_session_component *Validator_root::_create_session(const char *args)
{
	Validator_session_component *session =
				new (md_alloc()) Validator_session_component(_env, _md_alloc, args, _ts, _ds_size);

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
