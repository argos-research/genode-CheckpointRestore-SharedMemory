/*
 * \brief  Sizes for different allocations
 * \author Stephan Lex
 * \date   2017-06-22
 */

#ifndef _RTCR_UTIL_SIZES_H_
#define _RTCR_UTIL_SIZES_H_

namespace Rtcr {
	enum {
		ROOT_STACK_SIZE = 16*1024,
		EP_STACK_SIZE = 4*1024,
		DS_ALLOC_SIZE = 4*1024
	};
}

#endif /* _RTCR_UTIL_SIZES_H_ */
