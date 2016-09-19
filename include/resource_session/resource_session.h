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

	virtual void thread(Genode::Thread_capability thread_cap) = 0;
	virtual Genode::Dataspace_capability dataspace() = 0;

	/*******************
	 ** RPC interface **
	 *******************/

	GENODE_RPC(Rpc_thread, void, thread, Genode::Thread_capability);
	GENODE_RPC(Rpc_dataspace, Genode::Dataspace_capability, dataspace);
	GENODE_RPC_INTERFACE(Rpc_thread, Rpc_dataspace);
};

#endif /* _INCLUDE__RESOURCE_SESSION__RESOURCE_SESSION_H_ */
