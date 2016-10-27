/*
 * \brief  Checkpointer of Target_state
 * \author Denis Huber
 * \date   2016-10-26
 */

#include "checkpointer.h"

using namespace Rtcr;

void Checkpointer::_checkpoint_rm_sessions()
{
	Rm_root *rm_root = _child.rm_root();
	if(!rm_root) return;

	Genode::List<Rm_session_info> &child_infos = rm_root->rms_infos();
	Genode::List<Stored_rm_session_info> &state_infos = _state._stored_rm_sessions;

	Rm_session_info *child_info = child_infos.first();
	while(child_info)
	{
		Stored_rm_session_info *state_info = new (_state._alloc) Stored_rm_session_info();

		state_info->badge = child_info->session.cap().local_name();
		state_info->kcap  = _capability_map_infos.first()->find_by_badge(state_info->badge);
		state_info->args  = child_info->args;


		state_infos.insert(state_info);

		child_info = child_info->next();
	}
}

Checkpointer::Checkpointer(Target_child &child, Target_state &state)
:
	_child(child), _state(state), _capability_map_infos()
{ }

void Checkpointer::checkpoint()
{
	_checkpoint_rm_sessions();
}
