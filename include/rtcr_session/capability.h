/*
 * \brief  Rtcr-session capability type
 * \author Denis Huber
 * \date   2016-08-26
 */

#ifndef _INCLUDE__RTCR_SESSION__CAPABILITY_H_
#define _INCLUDE__RTCR_SESSION__CAPABILITY_H_

#include <base/capability.h>
#include <rtcr_session/rtcr_session.h>

namespace Rtcr { typedef Genode::Capability<Rtcr::Session> Session_capability; }

#endif /* _INCLUDE__RTCR_SESSION__CAPABILITY_H_ */
