/*
 * \brief  Restore mechanism for Target_child and Target_copy
 * \author Denis Huber
 * \date   2016-10-19
 */

#ifndef _RTCR_TARGET_RESTORER_H_
#define _RTCR_TARGET_RESTORER_H_

namespace Rtcr {
	class Target_restorer;

	constexpr bool restart_verbose_debug = true;
}

class Rtcr::Target_restorer
{
private:
	/**
	 * Enable log output for debugging
	 */
	static constexpr bool verbose_debug = restart_verbose_debug;

	Target_child &_child;
	Target_copy  &_copy;

	void _restore_threads();
	void _restore_capabilities();
	void _restore_region_maps();

	void _restore_region_map(Target_child &child, Genode::List<Copied_region_info> &copy_infos);

public:
	Target_restorer(Target_child &child, Target_copy &copy);
	void restore(Target_child &child, Target_copy &copy);

};

#endif /* _RTCR_TARGET_RESTORER_H_ */
