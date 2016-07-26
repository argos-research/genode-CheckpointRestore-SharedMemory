/*
 * \brief  Core-specific instance of the CR session interface
 * \author Denis Huber
 * \date   2016-07-25
 */

#ifndef _CORE__INCLUDE__CR_SESSION_COMPONENT_H_
#define _CORE__INCLUDE__CR_SESSION_COMPONENT_H_

/* Genode includes */
#include <util/string.h>
#include <base/log.h>
#include <base/rpc_server.h>
#include <cr_session/cr_session.h>

namespace Genode { class Cr_session_component; }

class Genode::Cr_session_component : public Rpc_object<Cr_session>
{
private:
    /**
    * Read and store the PD label
    */
    struct Label {

        enum { MAX_LEN = 64 };
        char string[MAX_LEN];

        Label(char const *args)
        {
            Arg_string::find_arg(args, "label").string(string,
                                                sizeof(string), "");
        }
    } const _label;
    Allocator *_alloced_pds;

public:
    Cr_session_component(Allocator *alloced_pds, char const *args)
    : _label(args),
      _alloced_pds(alloced_pds)
    { }
    

    /*************************
    ** CR session interface **
    *************************/

    bool checkpoint(String<64> label) 
    {
        log("Here arises a powerful checkpoint mechanism. Please wait.");
        return false; 
    }
    
    bool restore(String<64> label) 
    {
        log("Here arises a magnificent restore mechanism. Please be patient."); 
        return false;
    }
};

#endif /* _CORE__INCLUDE__CR_SESSION_COMPONENT_H_ */
