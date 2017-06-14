/*
 * \brief  Rtcr validator session interface
 * \author Stephan Lex
 * \date   2017-06-14
 */


#ifndef _INCLUDE__VALIDATOR_SESSION__VALIDATOR_SESSION_H_
#define _INCLUDE__VALIDATOR_SESSION__VALIDATOR_SESSION_H_

#include "capability.h"
#include <base/rpc_args.h>
#include <session/session.h>


namespace Rtcr { struct Validator_session; }


struct Rtcr::Validator_session : Genode::Session
{

	static const char *service_name() { return "VALIDATOR"; }


	virtual ~Validator_session() { }


	virtual bool dataspace_available() = 0;


	virtual Genode::Dataspace_capability get_dataspace() = 0;

	/*********************
	 ** RPC declaration **
	 *********************/


	GENODE_RPC(Rpc_dataspace_available, bool, dataspace_available);
	GENODE_RPC(Rpc_get_dataspace, Genode::Dataspace_capability, get_dataspace);

	GENODE_RPC_INTERFACE(Rpc_dataspace_available, Rpc_get_dataspace);
};



#endif /* _INCLUDE__VALIDATOR_SESSION__VALIDATOR_SESSION_H_ */
