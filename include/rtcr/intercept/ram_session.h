/*
 * \brief  Intercepting Ram session
 * \author Denis Huber
 * \date   2016-08-12
 */

#ifndef _RTCR_RAM_SESSION_H_
#define _RTCR_RAM_SESSION_H_

/* Genode includes */
#include <root/component.h>
#include <base/allocator.h>
#include <base/rpc_server.h>
#include <ram_session/ram_session.h>
#include <pd_session/connection.h>
#include <pd_session/pd_session.h>
#include <rm_session/connection.h>
#include <region_map/client.h>
#include <dataspace/client.h>
#include <util/retry.h>
#include <util/misc_math.h>
#include <os/packet_allocator.h>

/* Rtcr includes */
#include "../online_storage/ram_dataspace_info.h"
#include "../online_storage/ram_session_info.h"

namespace Rtcr {
	class Fault_handler;
	class Ram_session_component;
	class Ram_root;

	constexpr bool fh_verbose_debug = false;
	constexpr bool ram_verbose_debug = false;
	constexpr bool ram_root_verbose_debug = false;
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
	Genode::List<Ram_dataspace_info> &_ramds_infos;

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
	Fault_handler(Genode::Env &env, Genode::Signal_receiver &receiver,
			Genode::List<Ram_dataspace_info> &ramds_infos);

	/**
	 * Entrypoint of the thread
	 * The thread waits for a signal and calls the handler function if it receives any signal
	 */
	void entry();
};

/**
 * Custom RAM session to monitor the allocation, freeing, and ram quota transfers
 *
 * Instead of providing ordinary RAM dataspaces, it can provide managed dataspaces,
 * which are used to monitor the access to the provided RAM dataspaces
 */
class Rtcr::Ram_session_component : public Genode::Rpc_object<Genode::Ram_session>,
                                    private Genode::List<Ram_session_component>::Element
{
private:
	friend class Genode::List<Rtcr::Ram_session_component>;
	/**
	 * Enable log output for debugging
	 */
	static constexpr bool verbose_debug = ram_verbose_debug;

	/**
	 * Environment of Rtcr; needed to upgrade RAM quota of the RM session
	 */
	Genode::Env             &_env;
	/**
	 * Allocator for structures monitoring the allocated Ram dataspaces
	 */
	Genode::Allocator       &_md_alloc;
	/**
	 * Reference to Target_child's bootstrap phase
	 */
	bool                    &_bootstrap_phase;
	/**
	 * Connection to the parent Ram session (usually core's Ram session)
	 */
	Genode::Pd_connection   _parent_ram;
	/**
	 * Connection to the parent Rm session for creating new Region_maps (usually core's Rm session)
	 */
	Genode::Rm_connection    _parent_rm;
	/**
	 * State of parent's RPC object
	 */
	Ram_session_info         _parent_state;
	/**
	 * Receiver of page faults
	 */
	Genode::Signal_receiver  _receiver;
	/**
	 * Fault handler which marks and attaches dataspaces associated with the faulting address
	 */
	Fault_handler            _page_fault_handler;
	/**
	 * Size of Dataspaces which are associated with the managed dataspace
	 * _granularity is a multiple of a pagesize (4096 Byte)
	 * Zero means, no managed dataspaces are used
	 */
	Genode::size_t           _granularity;

	/**
	 * Destroy rds_info and all its sub infos)
	 */
	void _destroy_ramds_info(Ram_dataspace_info &rds_info);

public:
	Ram_session_component(Genode::Env &env, Genode::Allocator &md_alloc, Genode::size_t granularity,
			const char *label, const char *creation_args, bool &bootstrap_phase);
	~Ram_session_component();

	Genode::Ram_session_capability parent_cap() { return _parent_ram.cap(); }

	Ram_session_info &parent_state() { return _parent_state; }
	Ram_session_info const &parent_state() const { return _parent_state; }

	Ram_session_component *find_by_badge(Genode::uint16_t badge);

	using Genode::List<Rtcr::Ram_session_component>::Element::next;

	/***************************
	 ** Ram_session interface **
	 ***************************/

	/**
	 * Allocate a Ram_dataspace
	 *
	 * For managed Ram_dataspaces (_granularity > 0), create a new Region_map and allocate
	 * designated Ram_dataspaces for the Region_map and return the Ram_dataspace_capability of the
	 * Region_map.
	 */
	Genode::Ram_dataspace_capability alloc(Genode::size_t size, Genode::Cache_attribute cached) override;
	/**
	 * Frees the Ram_dataspace and destroys all monitoring structures
	 */
	void free(Genode::Ram_dataspace_capability ds_cap) override;

	Genode::size_t dataspace_size(Genode::Ram_dataspace_capability ds) const override;
	void assign_parent(Genode::Capability<Genode::Parent> parent) override;
	bool assign_pci(Genode::addr_t pci_config_memory_address, Genode::uint16_t bdf) override;
	void map(Genode::addr_t virt, Genode::addr_t size) override;
	Signal_source_capability alloc_signal_source() override;
	void free_signal_source(Signal_source_capability cap) override;
  	Genode::Signal_context_capability alloc_context(Signal_source_capability source, unsigned long imprint) override;
	void free_context(Genode::Capability<Genode::Signal_context> cap) override;
	void submit(Genode::Capability<Genode::Signal_context> context, unsigned cnt = 1) override;
	Genode::Native_capability alloc_rpc_cap(Genode::Native_capability ep) override;
	void free_rpc_cap(Genode::Native_capability cap) override;
	Genode::Capability<Genode::Region_map> address_space() override;
	Genode::Capability<Genode::Region_map> stack_area() override;
	Genode::Capability<Genode::Region_map> linker_area() override;
	void ref_account(Genode::Capability<Genode::Pd_session>) override;
	void transfer_quota(Genode::Capability<Genode::Pd_session> to, Genode::Cap_quota amount) override;
	Genode::Cap_quota cap_quota() const override;
	Genode::Cap_quota used_caps() const override;
	void transfer_quota(Genode::Capability<Genode::Pd_session> to, Genode::Ram_quota amount) override;
	Genode::Ram_quota ram_quota() const override;
	Genode::Ram_quota used_ram() const override;
	Genode::Capability<Genode::Pd_session::Native_pd> native_pd() override;
};



/**
 * Custom root RPC object to intercept session RPC object creation, modification, and destruction through the root interface
 */
class Rtcr::Ram_root : public Genode::Root_component<Ram_session_component>
{
private:
	/**
	 * Enable log output for debugging
	 */
	static constexpr bool verbose_debug = ram_root_verbose_debug;

	/**
	 * Environment of Rtcr; is forwarded to a created session object
	 */
	Genode::Env        &_env;
	/**
	 * Allocator for session objects and monitoring list elements
	 */
	Genode::Allocator  &_md_alloc;
	/**
	 * Entrypoint for managing session objects
	 */
	Genode::Entrypoint &_ep;
	/**
	 * Reference to Target_child's bootstrap phase
	 */
	bool               &_bootstrap_phase;
	/**
	 * Granularity of managed dataspaces
	 */
	Genode::size_t      _granularity;
	/**
	 * Lock for infos list
	 */
	Genode::Lock        _objs_lock;
	/**
	 * List for monitoring session objects
	 */
	Genode::List<Ram_session_component> _session_rpc_objs;

protected:
	Ram_session_component *_create_session(const char *args);
	void _upgrade_session(Ram_session_component *session, const char *upgrade_args);
	void _destroy_session(Ram_session_component *session);

public:
	Ram_root(Genode::Env &env, Genode::Allocator &md_alloc, Genode::Entrypoint &session_ep,
			Genode::size_t granularity, bool &bootstrap_phase);
    ~Ram_root();

	Genode::List<Ram_session_component> &session_infos() { return _session_rpc_objs; }
};


#endif /* _RTCR_RAM_SESSION_H_ */
