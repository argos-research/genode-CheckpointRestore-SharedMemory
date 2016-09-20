/*
 * \brief  Resource session client
 * \author Denis Huber
 * \date   2016-09-17
 */

#ifndef _INCLUDE__RESOURCE_REGISTRY_SESSION__CLIENT_H_
#define _INCLUDE__RESOURCE_REGISTRY_SESSION__CLIENT_H_

/* Genode includes */
#include <base/rpc_client.h>
#include <resource_session/capability.h>
#include <resource_session/resource_session.h>

namespace Resource { struct Session_client; }

struct Resource::Session_client : Genode::Rpc_client<Session>
{
	explicit Session_client(Session_capability session)
	: Rpc_client<Session>(session) { }

	virtual void provide(Genode::Native_capability cap, Genode::uint32_t id = 0) override {
		call<Rpc_provide>(cap, id); }

	virtual Genode::Native_capability request(Genode::uint32_t id = 0) override {
		return call<Rpc_request>(id); }
};

#endif /* _INCLUDE__RESOURCE_REGISTRY_SESSION__CLIENT_H_ */
