/*
 * \brief  Target's state
 * \author Denis Huber
 * \date   2016-10-25
 */

#include "target_state.h"

using namespace Rtcr;


template <typename T>
void Target_state::_delete_list(Genode::List<T>& infos)
{
	while(T* info = infos.first())
	{
		infos.remove(info);
		Genode::destroy(_alloc, info);
	}
}
// Template instanciation
template void Target_state::_delete_list(Genode::List<Stored_thread_info>& infos);


void Target_state::_delete_list(Genode::List<Stored_region_info>& infos)
{
	// Free RAM dataspaces and delete list elements
}


template <typename T>
void Target_state::_copy_list(Genode::List<T>& from_infos, Genode::List<T>& to_infos)
{
	// Exit, if the list is not empty
	if(to_infos.first()) return;

	T *from_info = from_infos.first();
	while(from_info)
	{
		T *to_info = new (_alloc) T(*from_info);
		to_infos.insert(to_info);

		from_info = from_info->next();
	}
}
// Template instanciation
template void Target_state::_copy_list(Genode::List<Stored_thread_info>& from_infos, Genode::List<Stored_thread_info>& to_infos);


Target_state::Target_state(Genode::Allocator& alloc)
:
	_alloc(alloc)
{ }


Target_state::Target_state(Target_state& other)
:
	_alloc(other._alloc)
{ }


Target_state::~Target_state()
{
	_delete_list(_stored_threads);
	_delete_list(_stored_address_space);
}
