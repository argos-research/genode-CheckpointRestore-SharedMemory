/*
 * \brief  CR root interface
 * \author Denis Huber
 * \date   2016-07-25
 */

#ifndef _CORE__INCLUDE__CR_ROOT_H_
#define _CORE__INCLUDE__CR_ROOT_H_

/* Genode */
#include <root/component.h>

/* Core */
#include <cr_session_component.h>

namespace Genode { class Cr_root; }

class Genode::Cr_root : public Genode::Root_component<Genode::Cr_session_component>
{
private:

protected:
    Genode::Cr_session_component *_create_session(const char *args)
    {
        /* Creating CR session */
        return new (md_alloc()) Genode::Cr_session_component(args);
    }
public:

    /**
     * Constructor
     * 
     * \param session_ep entrypoint for managing CR session objects
     * \param md_alloc   meta-data allocator to be used by root component
     */ 
    Cr_root(Genode::Rpc_entrypoint *session_ep, Genode::Allocator *md_alloc)
    : Genode::Root_component<Cr_session_component>(session_ep, md_alloc) { }
    
    virtual ~Cr_root() { }
};

#endif /* _CORE__INCLUDE__CR_ROOT_H_ */
