/*
 * \brief  Rtcr session
 * \author Denis Huber
 * \date   2016-08-05
 */

#ifndef _INCLUDE__RTCR_SESSION__RTCR_SESSION_H_
#define _INCLUDE__RTCR_SESSION__RTCR_SESSION_H_

#include <session/session.h>
#include <base/rpc.h>
#include <util/string.h>

namespace Rtcr { struct Session; }

struct Rtcr::Session : Genode::Session
{
	/*
	 * Exception types
	 */
	struct Exception : Genode::Exception { };
	struct Rom_module_does_not_exist : Exception { };

	typedef Genode::Rpc_in_buffer<64> Name;

	static const char *service_name() { return "Ckpt"; }

	virtual ~Session() { }

	virtual void checkpoint(Name const &component) = 0;
	virtual void restore(Name const &component) = 0;

	/*******************
	 ** RPC interface **
	 *******************/

	GENODE_RPC_THROW(Rpc_checkpoint, void, checkpoint, GENODE_TYPE_LIST(Rom_module_does_not_exist), Name const &);
	GENODE_RPC_THROW(Rpc_restore, void, restore, GENODE_TYPE_LIST(Rom_module_does_not_exist), Name const &);
	GENODE_RPC_INTERFACE(Rpc_checkpoint, Rpc_restore);
};

#endif /* _INCLUDE__RTCR_SESSION__RTCR_SESSION_H_ */
