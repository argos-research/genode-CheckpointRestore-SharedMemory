/*
 * \brief  Intercepting Pd session
 * \author Denis Huber
 * \date   2016-08-03
 */

#ifndef _RTCR_PD_SESSION_COMPONENT_H_
#define _RTCR_PD_SESSION_COMPONENT_H_

/* Genode includes */
#include <base/log.h>
#include <base/rpc_server.h>
#include <pd_session/connection.h>

/* Rtcr includes */
#include "region_map_component.h"
#include "../monitor/signal_source_info.h"
#include "../monitor/signal_context_info.h"
#include "../monitor/native_capability_info.h"

namespace Rtcr {
	class Pd_session_component;

	constexpr bool pd_verbose_debug = false;
}

/**
 * This virtual Pd session provides virtual Region maps
 */
class Rtcr::Pd_session_component : public Genode::Rpc_object<Genode::Pd_session>
{
private:
	/**
	 * Enable log output for debugging
	 */
	static constexpr bool verbose_debug = pd_verbose_debug;

	/**
	 * TODO Needed?
	 */
	Genode::Env           &_env;
	/**
	 * Allocator for list elements which monitor the Signal_source,
	 * Signal_context and Native_capability creation and destruction
	 */
	Genode::Allocator     &_md_alloc;
	/**
	 * Entrypoint to manage itself
	 */
	Genode::Entrypoint    &_ep;
	/**
	 * Connection to parent's pd session, usually from core
	 */
	Genode::Pd_connection  _parent_pd;
	/**
	 * Virtual address space for monitoring the attachments of the Region map
	 */
	Region_map_component   _address_space;
	/**
	 * Virtual stack area for monitoring the attachments of the Region map
	 */
	Region_map_component   _stack_area;
	/**
	 * Virtual linker area for monitoring the attachments of the Region map
	 */
	Region_map_component   _linker_area;

	/**
	 * Lock for Signal_source_info list
	 */
	Genode::Lock                         _ss_infos_lock;
	/**
	 * List for monitoring the creation and destruction of Signal_source_capabilities
	 */
	Genode::List<Signal_source_info>     _ss_infos;
	/**
	 * Lock for Signal_context_info list
	 */
	Genode::Lock                         _sc_infos_lock;
	/**
	 * List for monitoring the creation and destruction of Signal_context_capabilities
	 */
	Genode::List<Signal_context_info>    _sc_infos;
	/**
	 * Lock for Native_capability_info list
	 */
	Genode::Lock                         _nc_infos_lock;
	/**
	 * List for monitoring the creation and destruction of Native_capabilities
	 */
	Genode::List<Native_capability_info> _nc_infos;

public:
	/**
	 * Constructor
	 */
	Pd_session_component(Genode::Env &env, Genode::Allocator &md_alloc, Genode::Entrypoint &ep, const char *label);
	~Pd_session_component();

	Genode::Pd_session_capability         parent_cap()              { return _parent_pd.cap(); }
	Region_map_component                 &address_space_component() { return _address_space;   }
	Region_map_component                 &stack_area_component()    { return _stack_area;      }
	Region_map_component                 &linker_area_component()   { return _linker_area;     }
	Genode::List<Signal_source_info>     &signal_source_infos()     { return _ss_infos;        }
	Genode::List<Signal_context_info>    &signal_context_infos()    { return _sc_infos;        }
	Genode::List<Native_capability_info> &native_capability_infos() { return _nc_infos;        }

	/**************************
	 ** Pd_session interface **
	 **************************/

	void assign_parent(Genode::Capability<Genode::Parent> parent) override;
	bool assign_pci(Genode::addr_t addr, Genode::uint16_t bdf) override;
	Signal_source_capability alloc_signal_source() override;
	void free_signal_source(Signal_source_capability cap) override;
	Genode::Signal_context_capability alloc_context(Signal_source_capability source,
			unsigned long imprint) override;
	void free_context(Genode::Signal_context_capability cap) override;
	void submit(Genode::Signal_context_capability context, unsigned cnt) override;
	Genode::Native_capability alloc_rpc_cap(Genode::Native_capability ep) override;
	void free_rpc_cap(Genode::Native_capability cap) override;
	/**
	 * Return custom address space
	 */
	Genode::Capability<Genode::Region_map> address_space() override;
	/**
	 * Return custom stack area
	 */
	Genode::Capability<Genode::Region_map> stack_area() override;
	/**
	 * Return custom linker area
	 */
	Genode::Capability<Genode::Region_map> linker_area() override;
	Genode::Capability<Native_pd> native_pd() override;

};

#endif /* _RTCR_PD_SESSION_COMPONENT_H_ */
