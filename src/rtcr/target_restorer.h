/*
 * \brief  Restore mechanism for Target_child and Target_copy
 * \author Denis Huber
 * \date   2016-10-19
 */

#ifndef _RTCR_TARGET_RESTORER_H_
#define _RTCR_TARGET_RESTORER_H_

/* Genode includes */
#include <util/list.h>

/* Rtcr includes */
#include "target_copy.h"

namespace Rtcr {
	class Target_restorer;

	constexpr bool restore_verbose_debug = true;

	/* forward declaration */
	class Target_child;
}

class Rtcr::Target_restorer
{
private:
	/**
	 * Enable log output for debugging
	 */
	static constexpr bool verbose_debug = restore_verbose_debug;

	Target_child &_child;
	Target_copy  &_copy;

	void _restore_threads();
	void _restore_capabilities();
	void _restore_region_maps();

	void _restore_region_map(Genode::List<Attached_region_info> &orig_infos, Genode::List<Copied_region_info> &copy_infos);

public:
	Target_restorer(Target_child &child, Target_copy &copy);
	void restore();

};

#endif /* _RTCR_TARGET_RESTORER_H_ */
