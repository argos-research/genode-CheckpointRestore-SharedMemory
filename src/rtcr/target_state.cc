/*
 * \brief  Target's state
 * \author Denis Huber
 * \date   2016-10-25
 */

#include "target_state.h"

using namespace Rtcr;


template <typename T>
void Target_state::_delete_list(Genode::List<T> &infos)
{
	while(T* info = infos.first())
	{
		infos.remove(info);
		Genode::destroy(_alloc, info);
	}
}
template void Target_state::_delete_list(Genode::List<Stored_log_session_info> &infos);
template void Target_state::_delete_list(Genode::List<Stored_timer_session_info> &infos);
template void Target_state::_delete_list(Genode::List<Stored_thread_info> &infos);
template void Target_state::_delete_list(Genode::List<Stored_attached_region_info> &infos);
template void Target_state::_delete_list(Genode::List<Stored_signal_context_info> &infos);
template void Target_state::_delete_list(Genode::List<Stored_signal_source_info> &infos);


void Target_state::_delete_list(Genode::List<Stored_rm_session_info> &infos)
{
	while(Stored_rm_session_info* info = infos.first())
	{
		infos.remove(info);
		_delete_list(info->stored_region_map_infos);
		Genode::destroy(_alloc, info);
	}
}


void Target_state::_delete_list(Genode::List<Stored_region_map_info> &infos)
{
	while(Stored_region_map_info* info = infos.first())
	{
		infos.remove(info);
		_delete_list(info->stored_attached_region_infos);
		Genode::destroy(_alloc, info);
	}
}


void Target_state::_delete_list(Genode::List<Stored_dataspace_info> &infos)
{
	while(Stored_dataspace_info* info = infos.first())
	{
		infos.remove(info);
		_env.ram().free(Genode::static_cap_cast<Genode::Ram_dataspace>(info->ds_cap));
		Genode::destroy(_alloc, info);
	}
}

template <typename T>
void Target_state::_copy_list(Genode::List<T> &from_infos, Genode::List<T> &to_infos)
{
	// Remove list elements from to_infos
	_delete_list(to_infos);

	// Copy list elements from from_infos
	T *from_info = from_infos.first();
	while(from_info)
	{
		T *to_info = new (_alloc) T(*from_info);
		to_infos.insert(to_info);
		from_info = from_info->next();
	}
}
template void Target_state::_copy_list(Genode::List<Stored_log_session_info> &from_infos, Genode::List<Stored_log_session_info> &to_infos);
template void Target_state::_copy_list(Genode::List<Stored_timer_session_info> &from_infos, Genode::List<Stored_timer_session_info> &to_infos);
template void Target_state::_copy_list(Genode::List<Stored_thread_info> &from_infos, Genode::List<Stored_thread_info> &to_infos);
template void Target_state::_copy_list(Genode::List<Stored_attached_region_info> &from_infos, Genode::List<Stored_attached_region_info> &to_infos);
template void Target_state::_copy_list(Genode::List<Stored_signal_context_info> &from_infos, Genode::List<Stored_signal_context_info> &to_infos);
template void Target_state::_copy_list(Genode::List<Stored_signal_source_info> &from_infos, Genode::List<Stored_signal_source_info> &to_infos);


void Target_state::_copy_list(Genode::List<Stored_rm_session_info> &from_infos, Genode::List<Stored_rm_session_info> &to_infos)
{
	// Remove list elements from to_infos
	_delete_list(to_infos);

	// Copy list elements from from_infos
	Stored_rm_session_info *from_info = from_infos.first();
	while(from_info)
	{
		Stored_rm_session_info *to_info = new (_alloc) Stored_rm_session_info(*from_info);
		_copy_list(from_info->stored_region_map_infos, to_info->stored_region_map_infos);
		to_infos.insert(to_info);
		from_info = from_info->next();
	}
}


void Target_state::_copy_list(Genode::List<Stored_region_map_info> &from_infos, Genode::List<Stored_region_map_info> &to_infos)
{
	// Remove list elements from to_infos
	_delete_list(to_infos);

	// Copy list elements from from_infos
	Stored_region_map_info *from_info = from_infos.first();
	while(from_info)
	{
		Stored_region_map_info *to_info = new (_alloc) Stored_region_map_info(*from_info);
		_copy_list(from_info->stored_attached_region_infos, to_info->stored_attached_region_infos);
		to_infos.insert(to_info);
		from_info = from_info->next();
	}
}


void Target_state::_copy_list(Genode::List<Stored_dataspace_info> &from_infos, Genode::List<Stored_dataspace_info> &to_infos)
{
	// Remove list elements from to_infos
	_delete_list(to_infos);

	// Copy list elements from from_infos
	Stored_dataspace_info *from_info = from_infos.first();
	while(from_info)
	{
		// Create new dataspace and copy the contents of from_info to it
		Genode::Ram_dataspace_capability new_ds_cap = _env.ram().alloc(from_info->size);
		_copy_dataspace(from_info->ds_cap, new_ds_cap, from_info->size);

		Stored_dataspace_info *to_info = new (_alloc) Stored_dataspace_info(*from_info);
		to_info->ds_cap = new_ds_cap;

		to_infos.insert(to_info);

		from_info = from_info->next();
	}
}


void Target_state::_copy_dataspace(Genode::Dataspace_capability source_ds_cap, Genode::Dataspace_capability dest_ds_cap,
		Genode::size_t size, Genode::off_t dest_offset)
{
	char *source = _env.rm().attach(source_ds_cap);
	char *dest   = _env.rm().attach(dest_ds_cap);

	Genode::memcpy(dest + dest_offset, source, size);

	_env.rm().detach(dest);
	_env.rm().detach(source);
}


Target_state::Target_state(Genode::Env &env, Genode::Allocator &alloc)
:
	_env   (env),
	_alloc (alloc)
{ }


Target_state::Target_state(Target_state &other)
:
	_env   (other._env),
	_alloc (other._alloc)
{
	_copy_list(other._stored_rm_sessions,   _stored_rm_sessions);
	_copy_list(other._stored_log_sessions,  _stored_log_sessions);
	_copy_list(other._stored_threads,       _stored_threads);
	_copy_list(other._stored_address_space, _stored_address_space);
	_copy_list(other._stored_stack_area,    _stored_stack_area);
	_copy_list(other._stored_linker_area,   _stored_linker_area);
	_copy_list(other._stored_dataspaces,    _stored_dataspaces);
}


Target_state::~Target_state()
{
	_delete_list(_stored_rm_sessions);
	_delete_list(_stored_log_sessions);
	_delete_list(_stored_threads);
	_delete_list(_stored_address_space);
	_delete_list(_stored_stack_area);
	_delete_list(_stored_linker_area);
	_delete_list(_stored_dataspaces);
}

