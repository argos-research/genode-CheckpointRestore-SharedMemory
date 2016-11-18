/*
 * \brief  Restorer of Target_child
 * \author Denis Huber
 * \date   2016-10-26
 */

#ifndef _RTCR_RESTORER_H_
#define _RTCR_RESTORER_H_

#include "target_state.h"
#include "target_child.h"

namespace Rtcr {
	class Restorer;

	constexpr bool restorer_verbose_debug = true;
}

class Rtcr::Restorer
{
private:
	/**
	 * List element which translates a badge from the checkpoint process
	 * to a badge from the restoration process
	 */
	struct Badge_badge_info : Genode::List<Badge_badge_info>::Element
	{
		Genode::uint16_t ckpt_badge;
		Genode::uint16_t resto_badge;
		Badge_badge_info() : ckpt_badge(0), resto_badge(0) { }
		Badge_badge_info(Genode::uint16_t ckpt_badge, Genode::uint16_t resto_badge)
		: ckpt_badge(ckpt_badge), resto_badge(resto_badge) { }

		Badge_badge_info *find_by_ckpt_badge(Genode::uint16_t badge)
		{
			if(badge == ckpt_badge)
				return this;
			Badge_badge_info *info = next();
			return info ? info->find_by_ckpt_badge(badge) : 0;
		}
		Badge_badge_info *find_by_resto_badge(Genode::uint16_t badge)
		{
			if(badge == resto_badge)
				return this;
			Badge_badge_info *info = next();
			return info ? info->find_by_resto_badge(badge) : 0;
		}

		void print(Genode::Output &output) const
		{
			using Genode::Hex;

			Genode::print(output, "ckpt=", ckpt_badge, ", resto=", resto_badge);
		}
	};

	/**
	 * Enable log output for debugging
	 */
	static constexpr bool verbose_debug = restorer_verbose_debug;
	/**
	 * Allocator for restorer's personal datastructures. The datastructures which belong to Target_state
	 * are created with the allocator of Target_state
	 */
	Genode::Allocator &_alloc;
	Target_child &_child;
	Target_state &_state;
	Genode::List<Badge_badge_info> _badge_dictionary;


	void _restore_pd_session(Pd_session_component &child_obj, Stored_pd_session_info &state_info);
	void _restore_contexts(Pd_session_component &child_obj, Genode::List<Stored_signal_context_info> &state_infos);
	void _restore_sources(Pd_session_component &child_obj, Genode::List<Stored_signal_source_info> &state_infos);
	void _restore_region_map(Region_map_component &child_obj, Stored_region_map_info &state_info);
	void _restore_threads(Cpu_session_component &child_obj, Genode::List<Stored_thread_info> &state_infos);

	void _destroy_badge_dictionary();

public:
	Restorer(Genode::Allocator &alloc, Target_child &child, Target_state &state);
	~Restorer();

	void restore();
};

#endif /* _RTCR_RESTORER_H_ */
