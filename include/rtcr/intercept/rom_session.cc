/*
 * \brief  Intercepting ROM session
 * \author Denis Huber
 * \date   2016-10-24
 */

#include "rom_session.h"

using namespace Rtcr;


Rtcr::Rom_session_component::Rom_session_component(Genode::Env& env, Genode::Allocator& md_alloc, Genode::Rpc_entrypoint& ep,
		const char *label, const char *creation_args, bool &bootstrap_phase)
:
	_env          (env),
	_md_alloc     (md_alloc),
	_ep           (ep),
	_parent_rom   (env, label),
	_parent_state (creation_args, bootstrap_phase)
{
	ep.manage(this);
	//if(verbose_debug) Genode::log("\033[33m", "Rom", "\033[0m(parent ", _parent_rom,")");
}


Rtcr::Rom_session_component::~Rom_session_component()
{
	//if(verbose_debug) Genode::log("\033[33m", "~Rom", "\033[0m ", _parent_rom);
}


Rom_session_component *Rom_session_component::find_by_badge(Genode::uint16_t badge)
{
	if(badge == cap().local_name())
		return this;
	Rom_session_component *obj = next();
	return obj ? obj->find_by_badge(badge) : 0;
}


Genode::Rom_dataspace_capability Rtcr::Rom_session_component::dataspace()
{
	if(verbose_debug) Genode::log("Rom::\033[33m", __func__, "\033[0m()");
	auto result = _parent_rom.dataspace();
	_parent_state.dataspace = result;
	_parent_state.size = Genode::Dataspace_client(Genode::static_cap_cast<Genode::Dataspace>(result)).size();
	if(verbose_debug) Genode::log("  result: ", result);

	return result;
}


bool Rtcr::Rom_session_component::update()
{
	if(verbose_debug) Genode::log("Rom::\033[33m", __func__, "\033[0m()");
	auto result = _parent_rom.update();
	if(verbose_debug) Genode::log("  result: ", result);

	return result;
}


void Rtcr::Rom_session_component::sigh(Genode::Signal_context_capability sigh)
{
	if(verbose_debug) Genode::log("Rom::\033[33m", __func__, "\033[0m()");
	_parent_state.sigh = sigh;
	_parent_rom.sigh(sigh);
}


Rom_session_component *Rom_root::_create_session(const char *args)
{
	if(verbose_debug) Genode::log("Rom_root::\033[33m", __func__, "\033[0m(", args,")");

	// Extracting label from args
	char label_buf[128];
	Genode::Arg label_arg = Genode::Arg_string::find_arg(args, "label");
	label_arg.string(label_buf, sizeof(label_buf), "");

	// Revert ram_quota calculation, because the monitor needs the original session creation argument
	char ram_quota_buf[32];
	char readjusted_args[160];
	Genode::strncpy(readjusted_args, args, sizeof(readjusted_args));

	Genode::size_t readjusted_ram_quota = Genode::Arg_string::find_arg(readjusted_args, "ram_quota").ulong_value(0);
	readjusted_ram_quota = readjusted_ram_quota + sizeof(Rom_session_component) + md_alloc()->overhead(sizeof(Rom_session_component));

	Genode::snprintf(ram_quota_buf, sizeof(ram_quota_buf), "%zu", readjusted_ram_quota);
	Genode::Arg_string::set_arg(readjusted_args, sizeof(readjusted_args), "ram_quota", ram_quota_buf);

	// Create custom Rom_session
	Rom_session_component *new_session =
			new (_md_alloc) Rom_session_component(_env, _md_alloc, _ep.rpc_ep(), label_buf, readjusted_args, _bootstrap_phase);

	Genode::Lock::Guard lock(_objs_lock);
	_session_rpc_objs.insert(new_session);

	return new_session;
}


void Rom_root::_upgrade_session(Rom_session_component *session, const char *upgrade_args)
{
	if(verbose_debug) Genode::log("Rom_root::\033[33m", __func__, "\033[0m(session ", session->cap(),", args=", upgrade_args,")");

	char ram_quota_buf[32];
	char new_upgrade_args[160];

	Genode::strncpy(new_upgrade_args, session->parent_state().upgrade_args.string(), sizeof(new_upgrade_args));

	Genode::size_t ram_quota = Genode::Arg_string::find_arg(new_upgrade_args, "ram_quota").ulong_value(0);
	Genode::size_t extra_ram_quota = Genode::Arg_string::find_arg(upgrade_args, "ram_quota").ulong_value(0);
	ram_quota += extra_ram_quota;

	Genode::snprintf(ram_quota_buf, sizeof(ram_quota_buf), "%zu", ram_quota);
	Genode::Arg_string::set_arg(new_upgrade_args, sizeof(new_upgrade_args), "ram_quota", ram_quota_buf);

	session->parent_state().upgrade_args = new_upgrade_args;

	_env.parent().upgrade(Genode::Parent::Env::pd(), upgrade_args);
}


void Rom_root::_destroy_session(Rom_session_component *session)
{
	if(verbose_debug) Genode::log("Rom_root::\033[33m", __func__, "\033[0m(session ", session->cap(),")");

	_session_rpc_objs.remove(session);
	Genode::destroy(_md_alloc, session);
}


Rom_root::Rom_root(Genode::Env &env, Genode::Allocator &md_alloc, Genode::Entrypoint &session_ep,
		bool &bootstrap_phase)
:
	Root_component<Rom_session_component>(session_ep, md_alloc),
	_env              (env),
	_md_alloc         (md_alloc),
	_ep               (session_ep),
	_bootstrap_phase  (bootstrap_phase),
	_objs_lock        (),
	_session_rpc_objs ()
{
	if(verbose_debug) Genode::log("\033[33m", __func__, "\033[0m");
	//Genode::log("Rom root cap ",cap());
}

Rom_root::~Rom_root()
{
	while(Rom_session_component *obj = _session_rpc_objs.first())
	{
		_session_rpc_objs.remove(obj);
		Genode::destroy(_md_alloc, obj);
	}

	if(verbose_debug) Genode::log("\033[33m", __func__, "\033[0m");
}
