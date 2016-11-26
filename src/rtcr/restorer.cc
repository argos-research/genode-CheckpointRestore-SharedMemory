/*
 * \brief  Restorer of Target_child
 * \author Denis Huber
 * \date   2016-10-26
 */

#include "restorer.h"

using namespace Rtcr;


void Restorer::_identify_recreate_pd_sessions(Genode::List<Pd_session_component> &pd_sessions,
		Genode::List<Stored_pd_session_info> &stored_pd_sessions)
{
	// There shall be only PD session by now (only 1 PD session is created during bootstrap)
	Pd_session_component *bootstrapped_pd_session = pd_sessions.first();

	Pd_session_component *pd_session = nullptr;
	Stored_pd_session_info *stored_pd_session = nullptr;

	// Error
	if(!bootstrapped_pd_session)
	{
		Genode::error("There is no PD session in the list");
		throw Genode::Exception();
	}
	if(bootstrapped_pd_session->next()) Genode::warning("There are more than 1 PD session in the PD session list");

	stored_pd_session = stored_pd_sessions.first();
	while(stored_pd_session)
	{
		if(_corresponding_pd_sessions(*bootstrapped_pd_session, *stored_pd_session))
		{
			// Identified bootstrapped PD session
			pd_session = bootstrapped_pd_session;
		}
		else
		{
			// Create PD session
			Genode::Rpc_in_buffer<160> creation_args(stored_pd_session->creation_args.string());
			Genode::Session_capability pd_session_cap = _child.custom_services().pd_root->session(creation_args, Genode::Affinity());
			pd_session = _child.custom_services().pd_root->session_infos().first();
			if(pd_session) pd_session = pd_session->find_by_badge(pd_session_cap.local_name());
			if(!pd_session)
			{
				Genode::error("Could not find newly created PD session for ", pd_session_cap);
				throw Genode::Exception();
			}
		}

		_ckpt_to_resto_badges.insert(new (_alloc) Ckpt_resto_badge_info(stored_pd_session->badge, pd_session->cap().local_name()));

		//_identify_recreate_signal_sources(pd_session->parent_state().signal_sources, stored_pd_session->stored_source_infos);
		//_identify_recreate_signal_contexts(pd_session->parent_state().signal_contexts, stored_pd_session->stored_context_infos);
		// XXX postpone native caps creation, because it needs capabilities from CPU thread

		stored_pd_session = stored_pd_session->next();
	}

}
bool Restorer::_corresponding_pd_sessions(Pd_session_component &pd_session, Stored_pd_session_info &stored_pd_session)
{
	return pd_session.parent_state().bootstrapped && stored_pd_session.bootstrapped;
}







void Restorer::_restore_state_pd_sessions(Genode::List<Pd_session_component> &pd_sessions)
{
	Pd_session_component *pd_session = pd_sessions.first();
	while(pd_session)
	{
		// Upgrade session
		Genode::size_t ram_quota = Genode::Arg_string::find_arg(pd_session->parent_state().upgrade_args.string(), "ram_quota").ulong_value(0);
		if(ram_quota != 0) _state._env.parent().upgrade(pd_session->parent_cap(), pd_session->parent_state().upgrade_args.string());

		// No other state to restore

		pd_session = pd_session->next();
	}
}


Restorer::Restorer(Genode::Allocator &alloc, Target_child &child, Target_state &state)
: _alloc(alloc), _child(child), _state(state) { }


Restorer::~Restorer()
{

}


void Restorer::restore()
{
	if(verbose_debug) Genode::log("Resto::\033[33m", __func__, "\033[0m()");

	Genode::log("Before: \n", _child);

	// Identify or create RPC objects (caution: PD <- CPU thread <- Native cap ( "<-" means requires))
	//   Make a mapping of old badges to new badges
	// Restore state of all objects using mapping of old badges to new badges
	//   Also make a mapping of memory to restore

	// Replace old badges with new in capability map
	//   copy memory content from checkpointed dataspace which contains the cap map
	//   mark mapping from memory to restore as restored
	// Insert capabilities of all objects into capability space

	Genode::log("After: \n", _child);

}
