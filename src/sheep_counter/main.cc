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
#include <base/thread.h>
#include <base/sleep.h>
#include <timer_session/connection.h>
#include <cpu_thread/client.h>

class My_thread : public Genode::Thread
{
private:
	Genode::Env &_env;
public:
	My_thread(Genode::Env &env, const char *name, Genode::Cpu_session &cpu)
	:
		Thread(env, name, 0x2000, Thread::Location(), Thread::Weight(), cpu), _env(env)
    { }

	void entry()
	{
		int i = 0;
		Genode::log("Hello from thread ", name().string());
		while(1)
		{
			if(i >= 10) i--;
			else i++;
		}

	}
};

Genode::size_t Component::stack_size() { return 16*1024; }

void Component::construct(Genode::Env &env)
{
	using namespace Genode;
    unsigned int n = 1;
    Timer::Connection timer(env);
    /*
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
    */
    /*
    Ram_dataspace_capability ram_ds = env.ram().alloc(4096);
    char *local_addr = env.rm().attach(ram_ds);
    *local_addr = 'c';
    env.rm().detach(local_addr);
    */
    const char thread0_name[] = "Worker";
    My_thread thread0 { env, thread0_name, env.cpu() };
    thread0.start();

    timer.msleep(2000);

    Genode::Thread_state state { Genode::Cpu_thread_client(thread0.cap()).state() };
    Genode::log("Worker1: ip ", state.ip, ", sp ", state.sp);

    const char thread1_name[] = "Worker2";
    My_thread thread1 { env, thread1_name, env.cpu() };
    thread1.start();
    thread1.join();

    log("3 sheeps. zzZ");
}
