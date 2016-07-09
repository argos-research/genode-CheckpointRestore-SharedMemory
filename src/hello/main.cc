#include <base/printf.h>
#include <base/env.h>
#include <base/heap.h>
#include <base/slab.h>
#include <base/tslab.h>
#include <ram_session/ram_session.h>
#include <os/attached_ram_dataspace.h>

struct A 
{
    char _a[4];
    //A(char *a) : _a(a) { }
};

int main()
{
    Genode::printf("%s\n", "Hello world");
    
    /* RAM */
    Genode::printf("%s Session data\n", Genode::env()->ram_session()->service_name());
    Genode::printf("Quota = %d\n", Genode::env()->ram_session()->quota());
    Genode::printf("Used  = %d\n", Genode::env()->ram_session()->used());
    Genode::printf("Avail = %d\n\n", Genode::env()->ram_session()->avail());
    
    /* Dataspace */
    Genode::printf("Using Dataspace directly\n");
    Genode::Ram_dataspace_capability rds = Genode::env()->ram_session()->alloc(1024);
    char *dir_addr = Genode::env()->rm_session()->attach(rds);
    Genode::memset(dir_addr, 65, 3); // value 65, size 3
    Genode::printf("addr = %s\n", dir_addr);
    Genode::printf("Avail = %d\n\n", Genode::env()->ram_session()->avail());
    
    /* Attached_ram_dataspace */
    Genode::printf("Using Attached_ram_dataspace\n");
    Genode::Attached_ram_dataspace ard(Genode::env()->ram_session(), 1024);
    char *att_addr = ard.local_addr<char>();
    Genode::memset(att_addr, 66, 3);
    Genode::printf("att_addr = %s\n", att_addr);
    Genode::printf("Avail = %d\n\n", Genode::env()->ram_session()->avail());
    
    /* Heap */
    Genode::printf("Using Heap\n");
    Genode::printf("Heap consumed = %d\n", Genode::env()->heap()->consumed());
    Genode::printf("Creating struct A 2 times\n");
    struct A *heap_var1 = new (Genode::env()->heap()) A();
    struct A *heap_var2 = new (Genode::env()->heap()) A();
    Genode::printf("Created struct A on %p and on %p\n", heap_var1, heap_var2);
    Genode::printf("Heap consumed = %d\n", Genode::env()->heap()->consumed());
    Genode::printf("Destructing first struct A\n");
    Genode::env()->heap()->free(heap_var1, sizeof(struct A));
    Genode::printf("Pointers of struct A on %p and on %p\n", heap_var1, heap_var2);
    Genode::printf("Heap consumed = %d\n", Genode::env()->heap()->consumed());
    Genode::printf("Creating struct A\n");
    struct A *heap_var3 = new (Genode::env()->heap()) A();
    Genode::printf("Pointers of struct A on %p, on %p, and on %p\n", heap_var1, heap_var2, heap_var3);
    Genode::printf("Avail = %d\n\n", Genode::env()->ram_session()->avail());
    
    /* Sliced_heap */
    Genode::printf("Creating Sliced_heap\n");
    Genode::Sliced_heap sh(Genode::env()->ram_session(), Genode::env()->rm_session());
    Genode::printf("Sliced_heap consumed = %d\n", sh.consumed());
    Genode::printf("Creating struct A 8 times\n");
    for(int i = 0; i < 8; i++)
        struct A *var = new (&sh) A();
    Genode::printf("Sliced_heap consumed = %d\n", sh.consumed());
    Genode::printf("Avail = %d\n\n", Genode::env()->ram_session()->avail());
    
    /* Slab */
    Genode::printf("Creating Slab\n");
    Genode::Tslab<char, 4096> my_tslab(Genode::env()->heap());
    Genode::printf("Slab consumed = %d\n", my_tslab.consumed());
    Genode::printf("Creating struct A 2048 times\n");
    for(int i = 0; i < 2048; i++)
        struct A *var = new (&my_tslab) A();
    Genode::printf("Slab consumed = %d\n", my_tslab.consumed());
    Genode::printf("Avail = %d\n\n", Genode::env()->ram_session()->avail());
    
    
    return 0;
}
