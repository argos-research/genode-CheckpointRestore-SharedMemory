/*
 * \brief  Client-side validator session interface
 * \author Stephan Lex
 * \date   2017-06-14
 */


#ifndef _INCLUDE__VALIDATOR_SESSION__CLIENT_H_
#define _INCLUDE__VALIDATOR_SESSION__CLIENT_H_

#include "validator_session.h"
#include <base/rpc_client.h>

namespace Rtcr { struct Validator_session_client; }


struct Rtcr::Validator_session_client : Genode::Rpc_client<Validator_session>
{
	explicit Validator_session_client(Validator_session_capability session)
	: Rpc_client<Validator_session>(session)
	  { }

	bool dataspace_available() {
		return call<Rpc_dataspace_available>();	}

	Genode::Dataspace_capability get_dataspace() {
		return call<Rpc_get_dataspace>(); }

};

#endif /* _INCLUDE__VALIDATOR_SESSION__CLIENT_H_ */
