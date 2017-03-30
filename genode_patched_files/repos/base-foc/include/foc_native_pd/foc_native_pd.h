/*
 * \brief  Fiasco.OC-specific part of the PD session interface
 * \author Stefan Kalkowski
 * \author Norman Feske
 * \date   2011-04-14
 */

/*
 * Copyright (C) 2011-2016 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#ifndef _INCLUDE__FOC_NATIVE_PD__FOC_NATIVE_PD_H_
#define _INCLUDE__FOC_NATIVE_PD__FOC_NATIVE_PD_H_

#include <base/capability.h>
#include <base/rpc.h>
#include <pd_session/pd_session.h>

namespace Genode { struct Foc_native_pd; }


struct Genode::Foc_native_pd : Pd_session::Native_pd
{
	virtual Native_capability task_cap() = 0;
	// START Modification for Checkpoint/Restore (rtcr)
	virtual void install(Native_capability, addr_t) = 0;
	virtual addr_t cap_map_info() = 0;
	virtual void cap_map_info(addr_t) = 0;
	// END Modification for Checkpoint/Restore (rtcr)

	GENODE_RPC(Rpc_task_cap, Native_capability, task_cap);
	// START Modification for Checkpoint/Restore (rtcr)
	GENODE_RPC(Rpc_install, void, install, Native_capability, addr_t);
	GENODE_RPC(Rpc_get_cap_map_info, addr_t, cap_map_info);
	GENODE_RPC(Rpc_set_cap_map_info, void, cap_map_info, addr_t);
	GENODE_RPC_INTERFACE(Rpc_task_cap, Rpc_install, Rpc_get_cap_map_info, Rpc_set_cap_map_info);
	// END Modification for Checkpoint/Restore (rtcr)
};

#endif /* _INCLUDE__FOC_NATIVE_PD__FOC_NATIVE_PD_H_ */
