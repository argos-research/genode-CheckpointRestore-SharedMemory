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

	Genode::Env       &_env;
	Genode::Allocator &_alloc;

	Target_child      &_child;
	Target_copy       &_copy;

	void _restore_threads();
	void _restore_capabilities();
	void _restore_region_maps();

	/**
	 * \brief Restore Attached_region_infos and their dataspaces in the corresponding region map
	 *
	 * First, assign checkpointed dataspaces to child's dataspaces.
	 * Second, copy content of the
	 */
	void _restore_region_map(Genode::List<Attached_region_info> &orig_infos, Genode::List<Copied_region_info> &copy_infos);

public:
	Target_restorer(Genode::Env &env, Genode::Allocator &alloc, Target_child &child, Target_copy &copy);
	void restore();

};

#endif /* _RTCR_TARGET_RESTORER_H_ */
