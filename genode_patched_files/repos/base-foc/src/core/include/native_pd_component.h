/*
 * \brief  Kernel-specific part of the PD-session interface
 * \author Norman Feske
 * \date   2016-01-19
 */

/*
 * Copyright (C) 2016 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#ifndef _CORE__INCLUDE__NATIVE_PD_COMPONENT_H_
#define _CORE__INCLUDE__NATIVE_PD_COMPONENT_H_

/* Genode includes */
#include <foc_native_pd/foc_native_pd.h>

/* core-local includes */
#include <rpc_cap_factory.h>

namespace Genode {

	class Pd_session_component;
	class Native_pd_component;
}


class Genode::Native_pd_component : public Rpc_object<Foc_native_pd>
{
	private:

		Pd_session_component &_pd_session;
		addr_t                _cap_map_info;

	public:

		Native_capability task_cap() override;
		addr_t cap_map_info() override;
		void cap_map_info(addr_t addr) override;
		void install(Native_capability cap, addr_t kcap) override;

		Native_pd_component(Pd_session_component &pd, char const *args);

		~Native_pd_component();
};

#endif /* _CORE__INCLUDE__NATIVE_PD_COMPONENT_H_ */
