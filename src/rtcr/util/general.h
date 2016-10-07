/*
 * \brief  General helper functions
 * \author Denis Huber
 * \date   2016-08-17
 */

#ifndef _INCLUDE__RTCR_UTIL__GENERAL_H_
#define _INCLUDE__RTCR_UTIL__GENERAL_H_

#include <base/log.h>
#include <base/thread_state.h>
#include <util/list.h>

namespace Rtcr
{
	void dump_mem(const void *mem, unsigned int size);
	void print_thread_state(Genode::Thread_state &ts, bool brief = false);

	template<typename STRUCT_INFO>
	void print(Genode::Output &output, Genode::List<STRUCT_INFO> &list);
}



#endif /* _INCLUDE__RTCR_UTIL__GENERAL_H_ */
