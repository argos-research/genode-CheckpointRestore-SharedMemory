/*
 * \brief  Connection to Validator service
 * \author Stephan Lex
 * \date   2017-06-14
 */



#ifndef _INCLUDE__VALIDATOR_SESSION__CONNECTION_H_
#define _INCLUDE__VALIDATOR_SESSION__CONNECTION_H_

#include "client.h"
#include <base/connection.h>

namespace Rtcr { struct Validator_connection; }


struct Rtcr::Validator_connection : Genode::Connection<Rtcr::Validator_session>, Rtcr::Validator_session_client
{

	/**
	 * Issue session request
	 *
	 * \noapi
	 */
	Capability<Rtcr::Validator_session> _session(Genode::Parent &parent, char const *args)
	{
		return session(parent, args);
	}

	/**
	 * Constructor
	 *
	 * \param label     initial session label
	 */
	Validator_connection(Genode::Env &env, const char *args = "ram_quota=8K")
	:
		Genode::Connection<Rtcr::Validator_session>(env, _session(env.parent(), args)),
		Validator_session_client(cap())
	{ }

};

#endif /* _INCLUDE__VALIDATOR_SESSION__CONNECTION_H_ */
