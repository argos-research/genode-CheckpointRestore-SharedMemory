/*
 * \brief  Resource session capability type
 * \author Denis Huber
 * \date   2016-09-17
 */

#ifndef _INCLUDE__RESOURCE_SESSION__CAPABILITY_H_
#define _INCLUDE__RESOURCE_SESSION__CAPABILITY_H_

#include <base/capability.h>
#include <resource_session/resource_session.h>

namespace Resource { typedef Genode::Capability<Session> Session_capability; }

#endif /* _INCLUDE__RESOURCE_SESSION__CAPABILITY_H_ */
