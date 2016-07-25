/*
 * \brief  Checkpoint/Restore (CR) session interface
 * \author Denis Huber
 * \date   2016-07-25
 *
 * A Cr-session represents the Checkpoint/Restore mechanism of core.
 */

#ifndef _INCLUDE__CR_SESSION__CR_SESSION_H_
#define _INCLUDE__CR_SESSION__CR_SESSION_H_

namespace Genode {
    struct Cr_session;
}

struct Genode::Cr_session : Session
{
    static const char *service_name() { return "CR"; }

    virtual ~Cr_session() { }

    virtual void say_hello() = 0;
    virtual int add(int a, int b) = 0;

    GENODE_RPC(Rpc_say_hello, void, say_hello);
    GENODE_RPC(Rpc_add, int, add, int, int);
    GENODE_RPC_INTERFACE(Rpc_say_hello, Rpc_add);
};

#endif /* _INCLUDE__CR_SESSION__CR_SESSION_H_ */