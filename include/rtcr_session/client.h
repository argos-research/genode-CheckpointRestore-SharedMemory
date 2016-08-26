/*
 * \brief  Ckpt session client
 * \author Denis Huber
 * \date   2016-08-26
 */

#ifndef _INCLUDE__RTCR_SESSION__CLIENT_H_
#define _INCLUDE__RTCR_SESSION__CLIENT_H_

/* Genode includes */
#include <base/rpc_client.h>

/* Rtcr includes */
#include <rtcr_session/capability.h>
#include <rtcr_session/rtcr_session.h>

namespace Rtcr { struct Session_client; }

struct Rtcr::Session_client : Genode::Rpc_client<Rtcr::Session>
{
	explicit Session_client(Rtcr::Session_capability session)
	: Rpc_client<Session>(session) { }

	void checkpoint(Name const &name) override {
		call<Rpc_checkpoint>(name); }

	void restore(Name const &name) override {
		call<Rpc_restore>(name); }
};

#endif /* _INCLUDE__RTCR_SESSION__CLIENT_H_ */
