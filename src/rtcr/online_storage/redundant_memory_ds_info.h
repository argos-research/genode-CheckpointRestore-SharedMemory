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


struct Rtcr::Designated_redundant_ds_info: public Rtcr::Designated_dataspace_info, Genode::Lock
{

	/**
	 * Struct representing one checkpoint.
	 * The list tail is the active checkpoint that is used for redundant writing.
	 */
	struct Redundant_checkpoint : public Genode::List<Redundant_checkpoint>::Element
	{
	private:
		friend class Designated_redundant_ds_info;
		bool _attached;
		Genode::addr_t _addr;
		bool _is_cumulative;
		Genode::size_t _size;
		Genode::Region_map& _rm;
		Genode::Allocator& _alloc;
		//marks the bytes that have been modified in this snapshot
		Bitset* _written_bytes;
		Genode::addr_t* _bitset_array;
		static const Genode::size_t BITSET_UNIT_BITSIZE = sizeof(*_bitset_array)*8;
	public:

		Genode::Dataspace_capability const  red_ds_cap;
		/* If the snapshot is not cumulative, it is not a complete
		 * memory snapshot, i.e. some blocks refer to the previous
		 * snapshot.
		 */
		Redundant_checkpoint(Genode::Dataspace_capability const  ds_cap, Genode::size_t size,
				Genode::Allocator& alloc, Genode::Region_map& rm);

		~Redundant_checkpoint();

		Genode::addr_t attach();

		void detach();

		Genode::addr_t get_address();

		void print(Genode::Output &output) const;
	};

private:

	/* Head and tail of redundant checkpoint list.
	 * The tail is used for redundant writing.
	 * The head is needed for restoring or
	 * flattening checkpoints.
	 */
	Genode::List<Redundant_checkpoint> _checkpoints;
	Redundant_checkpoint* _active_checkpoint;

	Genode::Semaphore _sema_used_snapshots;

	Genode::Semaphore _sema_free_snapshots;

	Genode::Lock _lock;

	Genode::Ram_connection& _parent_ram;

	Genode::Cache_attribute _cached;

	Genode::Env& _env;

	Genode::Allocator& _alloc;

	Genode::Region_map& _rm;

	Genode::size_t _num_checkpoints;

	bool _redundant_writing;

	bool _new_checkpoint_pending;

	bool _primary_ds_attached_locally;

	Genode::addr_t _primary_ds_local_addr;

	Redundant_checkpoint* _allocated_snapshots[fixed_snapshot_amount];

	void _allocate_snapshots();

	void _create_new_checkpoint();

	void _flatten_previous_snapshots();

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

	bool primary_ds_attached_locally();

	Genode::addr_t attach_primary_ds_locally();

	void detach_primary_ds_locally();

	Genode::addr_t primary_ds_local_addr();

	bool redundant_writing();

	void redundant_writing(bool enable);

	void trigger_new_checkpoint();

	void new_checkpoint_if_pending();

	Redundant_checkpoint*  get_active_checkpoint();

	Genode::addr_t get_active_checkpoint_addr();

	Redundant_checkpoint* get_first_checkpoint();

	void copy_from_latest_checkpoint(void* dst);

	// dst is relative to dataspace start address
	void write_in_active_snapshot(Genode::addr_t dst, void* src, Genode::size_t data_size);

	void print(Genode::Output &output) const;

};


#endif /* _RTCR_REDUNDANT_MEMORY_DS_INFO_H_ */
