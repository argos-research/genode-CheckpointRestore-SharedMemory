/*
 * \brief  Checkpoint/Restore (CR) session interface
 * \author Denis Huber
 * \date   2016-07-25
 *
 * A CR session represents the Checkpoint/Restore mechanism of core.
 */

#ifndef _INCLUDE__CR_SESSION__CR_SESSION_H_
#define _INCLUDE__CR_SESSION__CR_SESSION_H_

#include <session/session.h>
#include <util/string.h>

namespace Genode {
    struct Cr_session;
}

struct Genode::Cr_session : Session
{
    static const char *service_name() { return "CR"; }

    virtual bool checkpoint(String<64> label) = 0;
    virtual bool restore(String<64> label) = 0;

    GENODE_RPC(Rpc_checkpoint, bool, checkpoint, String<64>);
    GENODE_RPC(Rpc_restore, bool, restore, String<64>);
    GENODE_RPC_INTERFACE(Rpc_checkpoint, Rpc_restore);
};

#endif /* _INCLUDE__CR_SESSION__CR_SESSION_H_ */
