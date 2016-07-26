/*
 * \brief  Test program for hello core client
 * \author Denis Huber
 * \date   2016-07-25
 *
 * Tests the initial implementation of a core service. Core provides a
 * new service (CR service) which offers the RPC methods say_hello and 
 * add
 */

#include <base/component.h>
#include <base/log.h>
#include <cr_session/connection.h>

Genode::size_t Component::stack_size() { return 16*4*1024; }

void Component::construct(Genode::Env &env)
{
    Genode::Cr_connection cr(env);
    
    cr.checkpoint("Hello");
    cr.restore("Hello");
    
    Genode::log("hello core test completed");
}
