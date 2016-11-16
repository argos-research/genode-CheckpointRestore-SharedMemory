/*
 * \brief  Client-side Fiasco.OC specific PD session interface
 * \author Stefan Kalkowski
 * \author Norman Feske
 * \date   2011-04-14
 */

/*
 * Copyright (C) 2011-2013 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#ifndef _INCLUDE__FOC_NATIVE_PD__CLIENT_H_
#define _INCLUDE__FOC_NATIVE_PD__CLIENT_H_

#include <foc_native_pd/foc_native_pd.h>
#include <base/rpc_client.h>

namespace Genode { struct Foc_native_pd_client; }


struct Genode::Foc_native_pd_client : Rpc_client<Foc_native_pd>
{
	explicit Foc_native_pd_client(Capability<Native_pd> cap)
	: Rpc_client<Foc_native_pd>(static_cap_cast<Foc_native_pd>(cap)) { }

	Native_capability task_cap() override { return call<Rpc_task_cap>(); }

	addr_t cap_map_info() override { return call<Rpc_get_cap_map_info>(); }

	void cap_map_info(addr_t addr) override { call<Rpc_set_cap_map_info>(addr); }

	void install(Native_capability cap, addr_t kcap) override { call<Rpc_install>(cap, kcap); }
};

#endif /* _INCLUDE__FOC_NATIVE_PD__CLIENT_H_ */
