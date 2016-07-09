/* Genode includes */
#include <base/printf.h>
#include <base/env.h>
#include <base/sleep.h>
#include <base/child.h> // includes: Rpc_entrypoint
#include <util/arg_string.h>
#include <ram_session/connection.h>
#include <rom_session/connection.h>
#include <cpu_session/connection.h>
#include <cap_session/connection.h>
#include <pd_session/connection.h>
#include <rm_session/connection.h>

class My_child : public Genode::Child_policy
{
private:
    /* Resources which will be transfered to the child */
    struct Resources
    {
        Genode::Pd_connection pd;
        Genode::Ram_connection ram;
        Genode::Cpu_connection cpu;
        Genode::Rm_connection rm;
        
        Resources(char const *label) : pd(label)
        {
            /* transfer some of our own ram quota to the new child */
            enum { CHILD_QUOTA = 1*1024*1024 };
            ram.ref_account(Genode::env()->ram_session_cap());
            Genode::env()->ram_session()->transfer_quota(ram.cap(), CHILD_QUOTA);
        }
    } _resources;
    
    /* 
     * Order is important. The services must appear before the child. 
     * Thus, the destruction will destruct the child first.
     */
     Genode::Rom_connection _rom;
     Genode::Parent_service _log_service;
     Genode::Parent_service _rm_service;
     Genode::Child          _child;
    
public:

    /**
     * Constructor
     */
    My_child(Genode::Rpc_entrypoint &ep, char const *elf_name)
    :
        _resources(elf_name),
        _rom(elf_name),
        _log_service("LOG"), _rm_service("RM"),
        _child(_rom.dataspace(), _resources.pd.cap(), _resources.ram.cap(),
               _resources.cpu.cap(), _resources.rm.cap(), &ep, this)
    { }
    
    /****************************
     ** Child-policy interface **
     ****************************/
     
    const char *name() const { return "my_test_child"; }
    
    Genode::Service *resolve_session_request(const char *service_name, const char *args)
    {
        /* forward these session requests to our parent (white-list concept) */
        return !Genode::strcmp(service_name, "LOG") ? &_log_service
             : !Genode::strcmp(service_name, "RM") ? &_rm_service
             : 0;
          
        /* Usage with Service_registry */     
        //static Genode::Service_registry parent_services;
        //parent_services.insert(new (env()->heap()) Genode::Parent_service("RAM"));
        //parent_services.insert(new (env()->heap()) Genode::Parent_service("ROM"));
        //return _parent_services->find(service_name);
    }
    
    void filter_session_args(const char *service, char *args, Genode::size_t args_len)
    {
        /* define session label for sessions forwarded to our parent */
        Genode::Arg_string::set_arg(args, args_len, "label", "hello");
    }
    
};

int main()
{
    Genode::printf("%s\n", "Launcher started");
    
    /*
     * Entrypoint used for serving the parent interface 
     * (does it mean that the parent uses the children as services?) 
     */
    enum { STACK_SIZE = 8*1024 };
    Genode::Cap_connection cap;
    Genode::Rpc_entrypoint ep(&cap, STACK_SIZE, "hello_ep");
    
    My_child child(ep, "hello");
    
    Genode::sleep_forever();
    
    return 0;
}
