/*
 * \brief  Structure for collecting capabilities and their creation arguments
 * \author Denis Huber
 * \date   2016-10-08
 */

#ifndef _RTCR_COPIED_CAP_COLL_COMPONENT_H_
#define _RTCR_COPIED_CAP_COLL_COMPONENT_H_

/* Genode includes */
#include <util/list.h>

/* Rtcr includes */
#include "../monitor/log_session_info.h"
#include "../monitor/rm_session_info.h"
#include "../monitor/timer_session_info.h"
#include "../monitor/native_capability_info.h"
#include "../monitor/ram_dataspace_info.h"
#include "../monitor/region_map_info.h"
#include "../monitor/signal_context_info.h"
#include "../monitor/signal_source_info.h"


namespace Rtcr {
	struct Copied_cap_coll;
}

struct Rtcr::Copied_cap_coll
{
	/*
	 * TODO Copy only necessary information instead of keeping shared objects between
	 * Rtcr component and target_child. If the child was checkpointed, it shall not change
	 * stored attributes!
	 */

	/* session RPC object caps */
	Genode::List<Log_session_info>   log_session_infos;
	Genode::List<Rm_session_info>    rm_session_infos;
	Genode::List<Timer_session_info> timer_session_infos;

	/* RPC object caps */
	Genode::List<Native_capability_info> native_cap_infos;
	Genode::List<Ram_dataspace_info>     ram_dataspace_infos;
	Genode::List<Region_map_info>        region_map_infos; // stores components, TODO store only necessary information
	Genode::List<Signal_context_info>    signal_context_infos;
	Genode::List<Signal_source_info>     signal_source_infos;

	Copied_cap_coll()
	:
		log_session_infos    (),
		rm_session_infos     (),
		timer_session_infos  (),
		native_cap_infos     (),
		ram_dataspace_infos  (),
		region_map_infos     (),
		signal_context_infos (),
		signal_source_infos  ()
	{ }
};


#endif /* _RTCR_COPIED_CAP_COLL_COMPONENT_H_ */
