/*
 * \brief  Rtcr session implementation
 * \author Denis Huber
 * \date   2016-08-26
 */

#ifndef _RTCR__RTCR_ROOT_H_
#define _RTCR__RTCR_ROOT_H_

/* Genode includes */
#include <root/component.h>

/* Rtcr includes */
#include "rtcr_session_component.h"

namespace Rtcr { class Root; }

class Rtcr::Root : public Genode::Root_component<Rtcr::Session_component>
{
	private:

	protected:

		Session_component *_create_session(const char *args)
		{
			return new (md_alloc()) Session_component();
		}

	public:

		/**
		 * Constructor
		 */
		Root(Genode::Entrypoint &session_ep, Genode::Allocator &md_alloc)
		:
			Root_component<Session_component>(session_ep, md_alloc)
		{ }
};


#endif /* _RTCR__RTCR_ROOT_H_ */
