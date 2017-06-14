/*
 * \brief  Validator-session capability type
 * \author Stephan Lex
 * \date   2017-06-14
 */



#ifndef _INCLUDE__VALIDATOR_SESSION__CAPABILITY_H_
#define _INCLUDE__VALIDATOR_SESSION__CAPABILITY_H_

#include <base/capability.h>

namespace Rtcr
{
	class Validator_session;
	typedef Genode::Capability<Validator_session> Validator_session_capability;
}

#endif /* _INCLUDE__VALIDATOR_SESSION__CAPABILITY_H_ */
