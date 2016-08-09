/*
 * \brief  Test program which just counts sheeps
 * \author Denis Huber
 * \date   2016-08-04
 *
 * This program is a target for the rtcr service. It counts a sheep,
 * prints the number of the sheep and goes to sleep between each 
 * iteration. This component will be checkpointed serialized, 
 * (transfered), deserialized, and restored. It does not know that it is
 * being checkpointed.
 */

#include <base/component.h>
#include <base/log.h>
#include <timer_session/connection.h>

Genode::size_t Component::stack_size() { return 32*4*1024; }

void Component::construct(Genode::Env &env)
{
	using namespace Genode;
    unsigned int n = 1;
    Timer::Connection timer(env);
    
    while(1)
    {
        if(n == 1)
            log("1 sheep. zzZ");
        else
            log(n, " sheeps. zzZ");
        n++;
        timer.msleep(2000);
        break;
    }
    Ram_dataspace_capability ram_ds = env.ram().alloc(4096);
    char *local_addr = env.rm().attach(ram_ds);
    *local_addr = 'c';
    log("3 sheeps. zzZ");
}
