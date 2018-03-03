/*
 * \brief  Rtcr child creation
 * \author Denis Huber
 * \date   2016-08-26
 */

/* Genode include */
#include <base/component.h>
#include <base/signal.h>
#include <base/sleep.h>
#include <base/log.h>
#include <base/child.h>
//#include <base/internal/elf.h>
#include <../src/include/base/internal/elf.h>
//#include <../src/include/base/internal/elf_format.h>
#include <timer_session/connection.h>

/* Rtcr includes */
#include "../../rtcr/target_child.h"
#include "../../rtcr/target_state.h"
#include "../../rtcr/checkpointer.h"
#include "../../rtcr/restorer.h"

namespace Rtcr {
struct Main;
}

struct Rtcr::Main {
	enum {
		ROOT_STACK_SIZE = 16 * 1024
	};
	Genode::Env &env;
	Genode::Heap heap { env.ram(), env.rm() };
	Genode::Service_registry parent_services { };

	Main(Genode::Env &env_) :
			env(env_) {
		using namespace Genode;

		Timer::Connection timer { env };

		Target_child child { env, heap, parent_services, "sheepcount", 1 };
		child.start();

		Target_state ts(env, heap);
		Checkpointer ckpt(heap, child, ts);

		while (1) {
			timer.msleep(5000);
			//int* bla = (int*)0x20000;
			//*bla=3;

			//child.pause();
			//ckpt.checkpoint();
			//timer.msleep(1000);

			//if(verbose_debug) Genode::log("Target_child::\033[33m", __func__, "\033[0m()");

			// Pause all threads of all sessions

			//resume() seems not reliable, thus call it multiple times
			//Cpu_thread_client c2;


			child.ram().parent_state().ram_dataspaces.first()->mrm_info->dd_infos.first()->detach();
			timer.msleep(3000);

			//instruction that is in the binary
//			unsigned int example_instruction = 0xe5843014;
			unsigned short example_instruction = 0x84e5;
			unsigned short current_instruction;

#if 1


			//TODO: TRY THIS
			//this->_ramds_infos.first()->mrm_info->dd_infos.first()->findbyaddr
			child.pause();
PINF("size of size_t: %u", sizeof(size_t));




#else
			Ram_dataspace_info* rinf = child.ram().parent_state().ram_dataspaces.first();
			PINF("looking for ds");


			while(rinf)
			{
				Designated_dataspace_info* ddinf = rinf->mrm_info->dd_infos.first();//->find_by_addr(state.pf_ip);
				while(ddinf)
				{
					char* addr = env.rm().attach(ddinf->cap);

					PINF("searching ds of size %u, first 4 bytes: %x", ddinf->size, *addr);

					for(unsigned long i = 0; i <= ddinf->size-sizeof(example_instruction); i++)
					{
						memcpy(&current_instruction,addr+i,sizeof(current_instruction));
						if(example_instruction == current_instruction)
						{
							PINF("FOUND IT!!!!!!!!!!!!!!!!!!!!!!!!!!!");
						}
					}

					//Genode::Dataspace ds(ddinf->cap);
					//this->
					//PINF("ds size: %lu", ds.size());
					//ddinf->cap


					//Designated_dataspace_info* t = ddinf->find_by_addr(state.pf_ip);
					//if(t)
					//	PINF("FOUND a dataspace");
					env.rm().detach(addr);
					PINF("didn't find yet a dataspace");
					ddinf = ddinf->next();
				}
				rinf = rinf->next();
			}
		/*	if(ddinf)
				PINF("found a dataspace.");
			else
				PINF("didn't find a dataspace");
*/


#endif
#if 0

			PINF("search space exhausted. Looking in ROM DSs");
			Rom_session_component* rsc = child.custom_services().rom_root->session_infos().first();
			while(rsc)
			{
				unsigned int* addr = env.rm().attach(rsc->dataspace());

				PINF("searching ds of size %u, first 4 bytes: %x", 0, *addr);

				for(unsigned long i = 0; i <= 16 /*rsc->size-sizeof(example_instruction)*/; i++)
				{
					if(example_instruction == * ((char*)addr+i))
					{
						PINF("FOUND IT!!!!!!!!!!!!!!!!!!!!!!!!!!!");
					}
				}

				//Genode::Dataspace ds(ddinf->cap);
				//this->
				//PINF("ds size: %lu", ds.size());
				//ddinf->cap


				//Designated_dataspace_info* t = ddinf->find_by_addr(state.pf_ip);
				//if(t)
				//	PINF("FOUND a dataspace");
				env.rm().detach(addr);
				PINF("didn't find yet a dataspace");

				rsc = rsc->next();
			}




#endif




/*
			Ram_dataspace_info* rdsi = child.ram().parent_state().ram_dataspaces.first();
			while(rdsi)
			{
				Designated_dataspace_info* ddi = rdsi->mrm_info->dd_infos.first();
				{
					ddi->detach();
					ddi = ddi->next();
				}
				rdsi = rdsi->next();
			}

*/
			// Iterate through every session
			Cpu_session_component *cpu_session = &child.cpu();
			//child._custom_services.cpu_root->session_infos().first();
/*			while (cpu_session) {

				// Iterate through every CPU thread
				Cpu_thread_component *cpu_thread =
						cpu_session->parent_state().cpu_threads.first();
				int j = 0;
				while (cpu_thread) {
					// Pause the CPU thread
					//Genode::Cpu_thread_client client{cpu_thread->parent_cap()};
					//client.pause();
					//if(cpu_thread->state().unresolved_page_fault)
					{

						PINF("Thread: %i, Pagefault: %lu, IP: %lx", j++,
								cpu_thread->state().unresolved_page_fault,
								cpu_thread->state().ip);
						Genode::Thread_state st = cpu_thread->state();
						//st.ip += 2;
						cpu_thread->state(st);

					}
					cpu_thread = cpu_thread->next();
				}

				cpu_session = cpu_session->next();
			}
*/
			// for(int i=0;i<10;i++)
			// 	child.resume();
		}

//      timer.msleep(3000);       
//		ckpt.checkpoint();

//		Target_child child_restored { env, heap, parent_services, "sheep_counter", 0 };
//		Restorer resto(heap, child_restored, ts);
//		child_restored.start(resto);

		//log("The End");
		Genode::sleep_forever();
	}
};

Genode::size_t Component::stack_size() {
	return 32 * 1024;
}

void Component::construct(Genode::Env &env) {
	static Rtcr::Main main(env);
}
