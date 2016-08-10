/*
 * \brief Cpu root interface
 * \author Denis Huber
 * \date 2016-08-10
 */

#ifndef _RTCR_CPU_ROOT_H_
#define _RTCR_CPU_ROOT_H_

/* Genode includes */
#include <root/component.h>

/* Rtcr includes */
#include "cpu_session_component.h"

namespace Rtcr {
	class Cpu_root;
}

class Rtcr::Cpu_root : public Genode::Root_component<Cpu_session_component>
{
private:
	const char *_label;
	/**
	 * Parent Pd cap for creating threads using parents pd cap (usually core's)
	 */
	Genode::Pd_session_capability _parent_pd_cap;

protected:

	Cpu_session_component *_create_session(const char *args)
	{
		return new (md_alloc())
				Cpu_session_component(_label, &ep(), &md_alloc(), _parent_pd_cap);
	}

public:

	Cpu_root(const char *label, Genode::Entrypoint &ep,	Genode::Allocator &alloc,
			Genode::Pd_session_capability parent_pd_cap)
	:
		Genode::Root_component<Cpu_session_component>(ep, alloc),
		_label(label), _parent_pd_cap(parent_pd_cap)
	{ }
};

#endif /* _RTCR_CPU_ROOT_H_ */
