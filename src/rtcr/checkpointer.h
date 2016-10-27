/*
 * \brief  Checkpointer of Target_state
 * \author Denis Huber
 * \date   2016-10-26
 */

#ifndef _RTCR_CHECKPOINTER_H_
#define _RTCR_CHECKPOINTER_H_

/* Genode includes */
#include <util/list.h>

/* Rtcr includes */
#include "target_state.h"

namespace Rtcr {
	class Checkpointer;
	struct Capability_map_info;

	constexpr bool checkpointer_verbose_debug = true;
}


struct Rtcr::Capability_map_info : Genode::List<Capability_map_info>::Element
{
	Genode::addr_t   kcap;
	Genode::uint16_t badge;

	Capability_map_info(Genode::addr_t kcap, Genode::uint16_t badge)
	: kcap(kcap), badge(badge) { }

	Capability_map_info *find_by_badge(Genode::uint16_t badge)
	{
		if(badge == this->badge)
			return this;
		Capability_map_info *info = next();
		return info ? info->find_by_badge(badge) : 0;
	}

	Capability_map_info *find_by_kcap(Genode::addr_t kcap)
	{
		if(kcap == this->kcap)
			return this;
		Capability_map_info *info = next();
		return info ? info->find_by_kcap(kcap) : 0;
	}
};


class Rtcr::Checkpointer
{
private:
	/**
	 * Enable log output for debugging
	 */
	static constexpr bool verbose_debug = checkpointer_verbose_debug;
	Target_child &_child;
	Target_state &_state;

	Genode::List<Capability_map_info> _capability_map_infos;

	void _checkpoint_rm_sessions();
	void _checkpoint_list(Genode::List<Rm_session_info> &child_infos, Genode::List<Stored_rm_session_info> &state_infos);

public:
	Checkpointer(Target_child &child, Target_state &state);

	void checkpoint();
};

#endif /* _RTCR_CHECKPOINTER_H_ */
