/*
 * \brief  Resource session
 * \author Denis Huber
 * \date   2016-09-17
 */

#ifndef _INCLUDE__RESOURCE_SESSION__RESOURCE_SESSION_H_
#define _INCLUDE__RESOURCE_SESSION__RESOURCE_SESSION_H_

#include <session/session.h>
#include <base/rpc.h>

namespace Resource { struct Session; }

struct Resource::Session : Genode::Session
{
	static const char *service_name() { return "Resource"; }

	virtual ~Session() { }

	virtual void provide(Genode::Native_capability cap, Genode::uint32_t id = 0) = 0;
	virtual Genode::Native_capability request(Genode::uint32_t id = 0) = 0;

	/*******************
	 ** RPC interface **
	 *******************/

	GENODE_RPC(Rpc_provide, void, provide, Genode::Native_capability, Genode::uint32_t);
	GENODE_RPC(Rpc_request, Genode::Native_capability, request, Genode::uint32_t);
	GENODE_RPC_INTERFACE(Rpc_provide, Rpc_request);
};

#endif /* _INCLUDE__RESOURCE_SESSION__RESOURCE_SESSION_H_ */
