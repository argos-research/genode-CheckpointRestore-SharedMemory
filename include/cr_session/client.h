/*
 * \brief  Client-side CR session interface
 * \author Denis Huber
 * \date   2016-07-25
 *
 * Description
 */

#ifndef _INCLUDE__CR_SESSION__CLIENT_H_
#define _INCLUDE__CR_SESSION__CLIENT_H_

#include <cr_session/capability.h>
#include <base/rpc_client.h>
#include <base/log.h>

namespace Genode { struct Cr_session_client; }


struct Genode::Cr_session_client : Rpc_client<Cr_session>
{
    explicit Cr_session_client(Cr_session_capability session_cap)
    : Rpc_client<Cr_session>(session_cap) { }
    
    virtual ~Cr_session_client() { }
    
    void say_hello()
    {
        log("issue RPC for saying hello.");
        call<Rpc_say_hello>();
        log("returned from 'say_hallo' RPC call.");
    }
    
    int add(int a, int b)
    {
        return call<Rpc_add>(a, b);
    }
};

#endif /* _INCLUDE__CR_SESSION__CLIENT_H_ */
