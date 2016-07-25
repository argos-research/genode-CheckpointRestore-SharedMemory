/*
 * \brief  CR session capability type
 * \author Denis Huber
 * \date   2016-07-25
 */

#ifndef _INCLUDE__CR_SESSION__CAPABILITY_H_
#define _INCLUDE__CR_SESSION__CAPABILITY_H_

#include <base/capability.h>
#include <cr_session/cr_session.h>

namespace Genode { typedef Capability<Cr_session> Cr_session_capability; }

#endif /* _INCLUDE__CR_SESSION__CAPABILITY_H_ */