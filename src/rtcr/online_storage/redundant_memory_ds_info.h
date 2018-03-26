#ifndef _RTCR_REDUNDANT_MEMORY_DS_INFO_H_
#define _RTCR_REDUNDANT_MEMORY_DS_INFO_H_

#include "../util/bitset.h"
#include "ram_dataspace_info.h"
#include <util/string.h>

namespace Rtcr {
	struct Designated_redundant_ds_info;
	constexpr bool redundant_memory_verbose_debug = true;
	constexpr bool verbose_register_debug = false;
	constexpr Genode::size_t fixed_snapshot_amount = 8;
#define FOC_RED_MEM_REGISTER_WORKAROUND
}


/**
 * Like Designated_dataspace_info, but additionally stores all neccessary information
 * for redundant memory, e.g. a list of memory snapshots.
 */
struct Rtcr::Designated_redundant_ds_info: public Rtcr::Designated_dataspace_info, Genode::Lock
{

	/**
	 * Struct representing one checkpoint.
	 */
	struct Redundant_checkpoint : public Genode::List<Redundant_checkpoint>::Element
	{
	private:
		friend class Designated_redundant_ds_info;
		/**
		 * If the snapshot is attached or not
		 */
		bool _attached;
		/**
		 * Local address of the snapshot
		 */
		Genode::addr_t _addr;
		/**
		 * When it is not cumulative, it means that it depends
		 * on previous snapshots.
		 */
		bool _is_cumulative;

		Genode::size_t _size;
		Genode::Region_map& _rm;
		Genode::Allocator& _alloc;
		/**
		 * marks the bytes that have been written/modified in this snapshot
		 */
		Bitset* _written_bytes;
		/**
		 * Backing array for _written_bytes
		 */
		Genode::addr_t* _bitset_array;
		static const Genode::size_t BITSET_UNIT_BITSIZE = sizeof(*_bitset_array)*8;
	public:

		/**
		 * Holds the dataspace of the redundant memory snapshot
		 */
		Genode::Dataspace_capability const  red_ds_cap;
		/**
		 * Constructor. Expects a valid dataspace as first argument
		 */
		Redundant_checkpoint(Genode::Dataspace_capability const  ds_cap, Genode::size_t size,
				Genode::Allocator& alloc, Genode::Region_map& rm);
		/**
		 * Destructor
		 */
		~Redundant_checkpoint();
		/**
		 * Attaches the snapshot locally (in Rtcr)
		 */
		Genode::addr_t attach();
		/**
		 * Detaches the snapshot locally
		 */
		void detach();
		/**
		 * Gets the local address
		 */
		Genode::addr_t get_address();
		void print(Genode::Output &output) const;
	};

private:

	/**
	 * List that contains all checkpoints currently in use.
	 * When creating a new checkpoint, this list temporarily
	 * grows. The flattener thread shrinks it.
	 */
	Genode::List<Redundant_checkpoint> _checkpoints;
	/**
	 * Points to the checkpoint that is currently used for
	 * redundant writing.
	 */
	Redundant_checkpoint* _active_checkpoint;
	/**
	 * Counts the number of snapshots, used for waking up
	 * the flattener thread
	 */
	Genode::Semaphore _sema_used_snapshots;
	/**
	 * Counts available snapshot slots, so that
	 * the snapshot-array isn't accessed out-of-bounds.
	 * Only used if fixed_snapshot_amount != 0,
	 * otherwise not necessary due to dynamic growth.
	 */
	Genode::Semaphore _sema_free_snapshots;
	/**
	 * Lock for thread synchronization
	 */
	Genode::Lock _lock;
	Genode::Ram_connection& _parent_ram;
	Genode::Cache_attribute _cached;
	Genode::Env& _env;
	Genode::Allocator& _alloc;
	Genode::Region_map& _rm;
	/**
	 * The number of currently used/occupied checkpoints
	 */
	Genode::size_t _num_checkpoints;
	/**
	 * If redundant writing is currently enabled or not
	 */
	bool _redundant_writing;
	/**
	 * If creation of a new checkpoint was requested or not
	 */
	bool _new_checkpoint_pending;
	/**
	 * If the primary ds (the one that target child itself uses)
	 * is currently attached locally (i.e. in Rtcr)
	 */
	bool _primary_ds_attached_locally;
	Genode::addr_t _primary_ds_local_addr;
	/**
	 * Pointers to the statically allocated snapshots.
	 * Not used when fixed_snapshot_amount == 0.
	 */
	Redundant_checkpoint* _allocated_snapshots[fixed_snapshot_amount];
	/**
	 * Statically pre-allocate snapshots
	 */
	void _allocate_snapshots();
	/**
	 * Create new checkpoint and redirect redundant writes to it
	 */
	void _create_new_checkpoint();
	/**
	 * Integrate all non-active snapshots (i.e. every one except the latest)
	 * into the first one and delete them.
	 */
	void _flatten_previous_snapshots();

	/**
	 * Thread that continuously waits for new
	 * snapshots and flattens them when appropriate
	 */
	class Flattener_thread : public Genode::Thread
	{
	private:
		Designated_redundant_ds_info* _parent;
	public:
		Flattener_thread(Designated_redundant_ds_info* parent);

		void entry();
	} _flattener_thread;

public:
	/**
	 * Constructor
	 */
	Designated_redundant_ds_info(Managed_region_map_info &mrm_info, Genode::Dataspace_capability ds_cap,
			Genode::addr_t addr, Genode::size_t size, Genode::Env& env, Genode::Allocator& alloc, Genode::Region_map& rm,
			Genode::Ram_connection& parent_ram, Genode::Cache_attribute cached);
	/**
	 * Destructor
	 */
	~Designated_redundant_ds_info();
	/**
	 * If the primary ds (the one that target child itself uses)
	 * is currently attached locally (i.e. in Rtcr)
	 */
	bool primary_ds_attached_locally();
	Genode::addr_t attach_primary_ds_locally();
	void detach_primary_ds_locally();
	Genode::addr_t primary_ds_local_addr();
	/**
	 * If redundant writing is currently enabled or not
	 */
	bool redundant_writing();
	/**
	 * Enable/disable redundant writing
	 */
	void redundant_writing(bool enable);
	/**
	 * From the next write access on, all redundant writes
	 * are redirected to a new snapshot and hereby a new
	 * checkpoint is created
	 */
	void trigger_new_checkpoint();
	/**
	 * If creation of a new checkpoint was requested, create it
	 */
	void new_checkpoint_if_pending();
	/**
	 * Get the checkpoint that is currently used for redundant writing
	 */
	Redundant_checkpoint*  get_active_checkpoint();
	/**
	 * Get local address of currently active checkpoint
	 */
	Genode::addr_t get_active_checkpoint_addr();
	/**
	 * Get the first checkpoint, i.e. the one that sooner
	 * or later will contain a cumulative snapshot of the
	 * latest checkpoint (due to flattener running in
	 * background)
	 */
	Redundant_checkpoint* get_first_checkpoint();
	/**
	 * Thread-safe method for restoring dataspace content
	 */
	void copy_from_latest_checkpoint(void* dst);
	/**
	 * Writes desired data into currently active snapshot
	 * and marks the respective bytes as changed. dst
	 * is the destination address relative to the dataspace
	 * start address.
	 */
	void write_in_active_snapshot(Genode::addr_t dst, void* src, Genode::size_t data_size);
	void print(Genode::Output &output) const;

};


#endif /* _RTCR_REDUNDANT_MEMORY_DS_INFO_H_ */
