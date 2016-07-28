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
#include <base/env.h>

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
    
    
    Sliced_heap *_alloced_pds;

public:
    Cr_session_component(Sliced_heap *alloced_pds, char const *args)
    : _label(args),
      _alloced_pds(alloced_pds)
    { }
    

    /*************************
    ** CR session interface **
    *************************/

    bool checkpoint(String<64> label)
    {
        log("Here arises a powerful checkpoint mechanism. Please wait.");
        Pd_session_component *pd = nullptr;
        bool found = false;
        Sliced_heap::Block *b = _alloced_pds->_blocks.first();
        for(; (b != 0) && !found; b = b->next())
        {
            //log("Block ", (void *) b, " B-size=", b->size);
            pd = reinterpret_cast<Pd_session_component*>(b + 1);
            //log("Pd label ", pd->_label.string);
            if(label == pd->_label.string)
            {
                found = true;
            }
        }
        
        log("Found=", found ? "true" : "false");
        if(found) log("Label=", pd->_label.string);
        
        Region_map_component rm_comp = pd->_address_space;
        /*Dataspace_client ds_client(rm_comp.);
        char* addr = core_env()->rm_session()->attach(rm_client.dataspace());
        
        log("Dataspace from PD");
        log("phys_addr=", ds_client.phys_addr(), " size=", ds_client.size(), " writable= ", ds_client.writable() ? "true" : "false");
*/

        return true;
    }
    
    bool restore(String<64> label) 
    {
        log("Here arises a magnificent restore mechanism. Please be patient."); 
        return false;
    }
};

#endif /* _CORE__INCLUDE__CR_SESSION_COMPONENT_H_ */
