/*
 * \brief  Core-specific instance of the CR session interface
 * \author Denis Huber
 * \date   2016-07-25
 */

#ifndef _CORE__INCLUDE__CR_SESSION_COMPONENT_H_
#define _CORE__INCLUDE__CR_SESSION_COMPONENT_H_

/* Genode includes */
#include <base/log.h>
#include <base/rpc_server.h>
#include <cr_session/cr_session.h>

namespace Genode { class Cr_session_component; }

class Genode::Cr_session_component : public Rpc_object<Cr_session>
{
private:

public:
    Cr_session_component(char const *args) { }
    virtual ~Cr_session_component() { }

    /*************************
    ** CR session interface **
    *************************/

    void say_hello() {
        log("I am here... Hello."); }
    int add(int a, int b) {
        return a+b; }
};

#endif /* _CORE__INCLUDE__CR_SESSION_COMPONENT_H_ */
