/*
 * \brief  Intercepting Ram session
 * \author Denis Huber
 * \date   2016-08-12
 */

#ifndef _RTCR_RAM_SESSION_COMPONENT_H_
#define _RTCR_RAM_SESSION_COMPONENT_H_

/* Genode includes */
#include <base/log.h>
#include <base/rpc_server.h>
#include <ram_session/connection.h>
#include <rm_session/connection.h>
#include <region_map/client.h>
#include <dataspace/client.h>
#include <util/retry.h>
#include <util/misc_math.h>

/* Rtcr includes */
#include "../monitor/ram_dataspace_info.h"

namespace Rtcr {
	class Fault_handler;
	class Ram_session_component;

	constexpr bool fh_verbose_debug = false;
	constexpr bool ram_verbose_debug = false;
}

/**
 * \brief Page fault handler designated to handle the page faults caused for the
 * incremental checkpoint mechanism of the custom Ram_session_component
 *
 * Page fault handler which has a list of region maps and their associated dataspaces.
 * Each page fault signal is handled by finding the faulting region map and attaching
 * the designated dataspace to the faulting address.
 */
class Rtcr::Fault_handler : public Genode::Thread
{
private:
	/**
	 * Enable log output for debugging
	 */
	static constexpr bool verbose_debug = fh_verbose_debug;
	/**
	 * Signal_receiver on which the page fault handler waits
	 */
	Genode::Signal_receiver          &_receiver;
	/**
	 * List of region maps and their associated dataspaces
	 * It must contain Managed_region_map_info
	 */
	Genode::List<Ram_dataspace_info> &_rds_infos;

	/**
	 * Find the first faulting Region_map in the list of Ram_dataspaces
	 *
	 * \return Pointer to Managed_region_map_info which contains the faulting Region_map
	 */
	Managed_region_map_info *_find_faulting_mrm_info();
	/**
	 * Handles the page fault by attaching a designated dataspace into its region map
	 */
	void _handle_fault();

public:
	/**
	 * Constructor
	 */
	Fault_handler(Genode::Env &env, Genode::Signal_receiver &receiver,
			Genode::List<Ram_dataspace_info> &rds_infos);

	/**
	 * Entrypoint of the thread
	 * The thread waits for a signal and calls the handler function if it receives any signal
	 */
	void entry();
};

/**
 * Virtual Ram_session to monitor the allocation, freeing, and ram quota transfers
 *
 * Instead of providing ordinary Ram_dataspaces, it can provide managed dataspaces,
 * which are used to monitor the access to the provided Ram_dataspaces
 */
class Rtcr::Ram_session_component : public Genode::Rpc_object<Genode::Ram_session>
{
private:
	/**
	 * Enable log output for debugging
	 */
	static constexpr bool verbose_debug = ram_verbose_debug;

	/**
	 * Environment of creator component (usually rtcr)
	 */
	Genode::Env                       &_env;
	/**
	 * Allocator for structures monitring the allocated Ram_dataspaces
	 */
	Genode::Allocator                 &_md_alloc;
	/**
	 * TODO Needed?
	 */
	Genode::Entrypoint                &_ep;
	/**
	 * Connection to the parent Ram session (usually core's Ram session)
	 */
	Genode::Ram_connection             _parent_ram;
	/**
	 * Connection to the parent Rm session for creating new Region_maps (usually core's Rm session)
	 */
	Genode::Rm_connection              _parent_rm;
	/**
	 * Lock to make the list of ram dataspaces thread-safe
	 */
	Genode::Lock                       _rds_infos_lock;
	/**
	 * List of ram dataspaces
	 */
	Genode::List<Ram_dataspace_info>   _rds_infos;
	/**
	 * Receiver of page faults
	 */
	Genode::Signal_receiver            _receiver;
	/**
	 * Fault handler which marks and attaches dataspaces associated with the faulting address
	 */
	Fault_handler                      _page_fault_handler;
	/**
	 * Size of Dataspaces which are associated with the managed dataspace
	 * _granularity is a multiple of a pagesize (4096 Byte)
	 * Zero means, no managed dataspaces are used
	 */
	Genode::size_t                     _granularity;

	/**
	 * Destroy rds_info and all its sub infos (unsynchronized)
	 */
	void _destroy_rds_info(Ram_dataspace_info &rds_info);

public:

	/**
	 * Constructor
	 */
	Ram_session_component(Genode::Env &env, Genode::Allocator &md_alloc, Genode::Entrypoint &ep,
			const char *name, Genode::size_t granularity = 1);
	/**
	 * Destructor
	 */
	~Ram_session_component();

	Genode::Ram_session_capability    parent_cap()          { return _parent_ram.cap(); }
	Genode::List<Ram_dataspace_info> &ram_dataspace_infos() { return _rds_infos;        }

	/***************************
	 ** Ram_session interface **
	 ***************************/

	/**
	 * Allocate a Ram_dataspace
	 *
	 * For managed Ram_dataspaces (_use_inc_ckpt == true), create a new Region_map and allocate
	 * designated Ram_dataspaces for the Region_map and return the Ram_dataspace_capability of the
	 * Region_map.
	 */
	Genode::Ram_dataspace_capability alloc(Genode::size_t size, Genode::Cache_attribute cached) override;
	/**
	 * Frees the Ram_dataspace and destroys all monitoring structures
	 */
	void free(Genode::Ram_dataspace_capability ds_cap) override;
	int ref_account(Genode::Ram_session_capability ram_session) override;
	int transfer_quota(Genode::Ram_session_capability ram_session, Genode::size_t amount) override;
	Genode::size_t quota() override;
	Genode::size_t used() override;

};

#endif /* _RTCR_RAM_SESSION_COMPONENT_H_ */
