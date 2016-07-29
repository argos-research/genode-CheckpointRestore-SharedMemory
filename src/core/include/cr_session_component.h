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
#include <region_map/client.h>
#include <dataspace/client.h>
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
        
        // Capabilities
/*
        Capability<Region_map> rm_cap = pd->address_space();
        Region_map_client rm_client(rm_cap);
        char* addr = core_env()->rm_session()->attach(rm_client.dataspace());
*/

        // Rpc_object directly
/*
        Dataspace_capability ds_cap = pd->address_space_region_map().dataspace();
        char* addr = core_env()->rm_session()->attach(ds_cap);
        log("Attaching dataspace: Returned pointer: ", addr);

        Dataspace_component *ds_comp = pd->address_space_region_map().dataspace_component();
        log("Reading dataspace component");
        log("phys_addr = ", ds_comp->phys_addr(), " size = ", ds_comp->size(), " writable = ", ds_comp->writable() ? "true" : "false");
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
