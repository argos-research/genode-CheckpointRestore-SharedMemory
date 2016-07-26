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
#include <util/string.h>

namespace Genode { struct Cr_session_client; }


struct Genode::Cr_session_client : Rpc_client<Cr_session>
{
    explicit Cr_session_client(Cr_session_capability session_cap)
    : Rpc_client<Cr_session>(session_cap) { }
    
    virtual ~Cr_session_client() { }
    
    bool checkpoint(String<64> label)
    {
        bool result;
        log("issue RPC for checkpoint(", label.string(), ").");
        result = call<Rpc_checkpoint>(label);
        log("returned from 'checkpoint' RPC call.");
        return result;
    }
    
    bool restore(String<64> label)
    {
        bool result;
        log("issue RPC for restore(", label.string(), ").");
        result = call<Rpc_restore>(label);
        log("returned from 'restore' RPC call.");
        return result;
    }
};

#endif /* _INCLUDE__CR_SESSION__CLIENT_H_ */
