/*
 * \brief  Connection to Resource service
 * \author Denis Huber
 * \date   2016-09-17
 */

#ifndef _INCLUDE__RESOURCE_SESSION__CONNECTION_H_
#define _INCLUDE__RESOURCE_SESSION__CONNECTION_H_

#include <resource_session/client.h>
#include <base/connection.h>

namespace Resource { struct Connection; }


struct Resource::Connection : Genode::Connection<Session>, Session_client
{
	/**
	 * Constructor
	 */
	Connection(Genode::Env &env)
	:
		Genode::Connection<Session>(env, session(env.parent(), "ram_quota=4K")),
		Session_client(cap())
	{ }
};

#endif /* _INCLUDE__RESOURCE_SESSION__CONNECTION_H_ */
