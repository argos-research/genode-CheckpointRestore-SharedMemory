/*
 * \brief  Connection to CR service
 * \author Denis Huber
 * \date   2016-07-25
 */

#ifndef _INCLUDE__CR_SESSION__CONNECTION_H_
#define _INCLUDE__CR_SESSION__CONNECTION_H_

#include <cr_session/client.h>
#include <base/connection.h>

namespace Genode { struct Cr_connection; }

struct Genode::Cr_connection : Connection<Cr_session>, Cr_session_client
{
    enum { RAM_QUOTA = 16*4*1024 };
    
    /**
     * Constructor
     * 
     * \param env    Environment of the component
     * \param label  Session label
     */ 
    Cr_connection(Env &env, char const *label = "")
    : Connection<Cr_session>(env, session(env.parent(), 
                                          "ram_quota=%u, label=\"%s\"",
                                          RAM_QUOTA, label)),
      Cr_session_client(cap())
    { }
    
};

#endif /* _INCLUDE__CR_SESSION__CONNECTION_H_ */
