#include <base/printf.h>
#include <base/env.h>
#include <ram_session/ram_session.h>

class A {};

int main()
{
    Genode::printf("%s\n", "Hello world");
    
    Genode::printf("%s\n", Genode::env()->ram_session()->service_name());
    Genode::printf("Quota = %d\n", Genode::env()->ram_session()->quota());
    Genode::printf("Used  = %d\n", Genode::env()->ram_session()->used());
    Genode::printf("Avail = %d\n", Genode::env()->ram_session()->avail());
    
    enum { QUOTA = 1024 };
    Genode::printf("Using %d B quota\n", QUOTA);
    Genode::Ram_dataspace_capability rds = Genode::env()->ram_session()->alloc(QUOTA);
    
    Genode::printf("Avail = %d\n", Genode::env()->ram_session()->avail());
    
    Genode::env()->rm_session()->attach(rds);
    Genode::printf("Attached ram dataspace to rm session\n");
    
    Genode::printf("RM session state = %d\n", Genode::env()->rm_session()->state());
    
    //Genode::printf("Avail = %d\n", Genode::env()->ram_session()->avail());
    
    return 0;
}
