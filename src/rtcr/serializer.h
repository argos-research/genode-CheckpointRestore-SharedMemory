/*
 * \brief  Serializer for component's data
 * \author Denis Huber
 * \date   2016-08-30
 */

#ifndef _RTCR__SERIALIZER_H_
#define _RTCR__SERIALIZER_H_

#include <base/attached_ram_dataspace.h>
#include <cpu_thread/client.h>

namespace Rtcr {
	class Serializer;
}

class Rtcr::Serializer
{
private:
	Genode::List<Rtcr::Thread_info> &_threads;
	Genode::List<Rtcr::Region_info> &_regions;
	Genode::Env &_env;
	Genode::Attached_ram_dataspace *_ds;
	/**
	 * Shows to the next free memory space
	 */
	void *_buf_ptr;

	Genode::size_t _calculate_buffer_size()
	{
		Genode::size_t result = 0;

		Rtcr::Thread_info *curr_thread = _threads.first();
		for( ; curr_thread; curr_thread = curr_thread->next())
		{
			result += THREAD_SIZE;
		}

		Rtcr::Region_info *curr_region = _regions.first();
		for( ; curr_region; curr_region = curr_region->next())
		{
			result += REGION_HEADER_SIZE + curr_region->size;
		}

		// Round up to full pages
		Genode::size_t num_pages = result / 4096;
		Genode::size_t rest_page = result % 4096;
		num_pages = num_pages + (rest_page != 0 ? 1 : 0);
		result = 4096 * num_pages;

		return result;
	}

	/**
	 * Serialize a thread to the dataspace
	 *
	 * \param thread Thread to serialize
	 *
	 * \return Bytes consumed in the dataspace
	 */
	Genode::size_t _serialize(Rtcr::Thread_info &thread)
	{
		Genode::Cpu_thread_client client {thread.thread_cap};
		Genode::Thread_state ts {client.state()};

		if(!ts.paused)
		{
			Genode::warning(" Thread ", thread.thread_cap.local_name(), " not paused.");
			return 0;
		}

		// Store registers and adjust pointer
		Genode::addr_t* addr_ptr = static_cast<Genode::addr_t*>(_buf_ptr);

		// Using post-increment:
		// Store register value to pointer, then increment pointer
		*(addr_ptr++) = ts.r0;
		*(addr_ptr++) = ts.r1;
		*(addr_ptr++) = ts.r2;
		*(addr_ptr++) = ts.r3;
		*(addr_ptr++) = ts.r4;
		*(addr_ptr++) = ts.r5;
		*(addr_ptr++) = ts.r6;
		*(addr_ptr++) = ts.r7;
		*(addr_ptr++) = ts.r8;
		*(addr_ptr++) = ts.r9;
		*(addr_ptr++) = ts.r10;
		*(addr_ptr++) = ts.r11;
		*(addr_ptr++) = ts.r12;
		*(addr_ptr++) = ts.sp;
		*(addr_ptr++) = ts.lr;
		*(addr_ptr++) = ts.ip;
		*(addr_ptr++) = ts.cpsr;
		*(addr_ptr++) = ts.lr;

		Genode::size_t diff = addr_ptr - static_cast<Genode::addr_t*>(_buf_ptr);

		_buf_ptr = addr_ptr;

		return diff;
	}

	/**
	 * Serialize all threads in the list to the dataspace
	 *
	 * \param  threads List of threads
	 *
	 * \return Bytes consumed in the dataspace
	 */
	Genode::size_t _serialize(Genode::List<Rtcr::Thread_info> &threads)
	{
		Genode::size_t diff = 0;

		Rtcr::Thread_info *curr_thread = _threads.first();
		for( ; curr_thread; curr_thread = curr_thread->next())
		{
			diff += _serialize(*curr_thread);
		}

		return diff;
	}

public:
	enum {THREAD_SIZE = 18*sizeof(Genode::addr_t), REGION_HEADER_SIZE = 0x10};


	Serializer(Genode::List<Rtcr::Thread_info> &threads, Genode::List<Rtcr::Region_info> &regions, Genode::Env &env)
	:
		_threads(threads),
		_regions(regions),
		_env(env),
		_ds(_env.ram(), _env.rm(), _calculate_buffer_size()),
		_buf_ptr(_ds->local_addr<void>())
	{ }
};

#endif /* _RTCR__SERIALIZER_H_ */
