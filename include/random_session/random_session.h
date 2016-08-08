/*
 * \brief  Test session
 * \author Denis Huber
 * \date   2016-08-05
 */

#ifndef _INCLUDE__RANDOM_SESSION__RANDOM_SESSION_H_
#define _INCLUDE__RANDOM_SESSION__RANDOM_SESSION_H_

#include <session/session.h>
#include <base/rpc.h>

namespace Random { struct Session; }

struct Random::Session : Genode::Session
{
	static const char *service_name() { return "Random"; }

	virtual void say_hello() = 0;

	/*******************
	 ** RPC interface **
	 *******************/

	GENODE_RPC(Rpc_say_hello, void, say_hello);
	GENODE_RPC_INTERFACE(Rpc_say_hello);
};

#endif /* _INCLUDE__RANDOM_SESSION__RANDOM_SESSION_H_ */
