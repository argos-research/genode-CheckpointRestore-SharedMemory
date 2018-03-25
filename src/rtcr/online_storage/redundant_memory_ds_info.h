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
				Genode::Allocator& alloc, Genode::Region_map& rm)
		:
			_attached(false), _addr(0), _is_cumulative(false),
			_size(size), _rm(rm), _alloc(alloc), red_ds_cap(ds_cap)
		{
			_bitset_array = new(_alloc) Genode::addr_t[_size/BITSET_UNIT_BITSIZE];
			_written_bytes = new(_alloc) Bitset((_size + BITSET_UNIT_BITSIZE - 1)
					& ~(BITSET_UNIT_BITSIZE - 1), _bitset_array, true);
		}

		~Redundant_checkpoint()
		{
			Genode::destroy(_alloc, _written_bytes);
			Genode::destroy(_alloc, _bitset_array);
		}

		Genode::addr_t attach()
		{
			if(_attached)
			{
				PWRN("Trying to re-attach already attached redundant memory checkpoint");
				return _addr;
			}
			_addr = _rm.attach(red_ds_cap);
			_attached = true;
			return _addr;
		}

		void detach()
		{
			if(!_attached)
			{
				PWRN("Trying to detach a non-attached redundant memory checkpoint");
				return;
			}
			_rm.detach(_addr);
			_addr = 0;
			_attached = false;
		}

		Genode::addr_t get_address()
		{
			return _addr;
		}

		void print(Genode::Output &output) const
		{
			Genode::print(output, "\n    Changes in snapshot ", red_ds_cap, "\t->\t");
			for(Genode::addr_t i = 0; i<_size; i += 1)
			{
				if(_written_bytes->get(i,1))
					Genode::print(output, Genode::Hex(i), ": ",
							Genode::Hex(*(Genode::uint8_t*) (_addr + i)), "; ");
			}
			Genode::print(output, "next: ", Genode::Hex((Genode::uint64_t) this->_next));
		}
	};

private:

	/* Head and tail of redundant checkpoint list.
	 * The tail is used for redundant writing.
	 * The head is needed for restoring or
	 * flattening checkpoints.
	 */
	Genode::List<Redundant_checkpoint> _checkpoints;
	Redundant_checkpoint* _active_checkpoint;

	Genode::Semaphore _sema;

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

	void _allocate_snapshots()
	{
		for(Genode::size_t i=0; i<fixed_snapshot_amount; i++)
		{
			Genode::Dataspace_capability const ds = _parent_ram.alloc(size,_cached);
			Redundant_checkpoint* new_checkpoint = new (_alloc) Redundant_checkpoint(ds, size, _alloc, _rm);
			_allocated_snapshots[i] = new_checkpoint;
		}
	}

	void _create_new_checkpoint()
	{
		Redundant_checkpoint* new_checkpoint;

		if(!fixed_snapshot_amount)
		{
			Genode::Dataspace_capability const ds = _parent_ram.alloc(size,_cached);
			new_checkpoint = new (_alloc) Redundant_checkpoint(ds, size, _alloc, _rm);
		}
		else
		{
			new_checkpoint = _allocated_snapshots[_num_checkpoints];
			Genode::memset(new_checkpoint->_bitset_array, 0, size/Redundant_checkpoint::BITSET_UNIT_BITSIZE);
			new_checkpoint->_is_cumulative = false;
			new_checkpoint->_next = nullptr;
		}
		new_checkpoint->attach();
		_checkpoints.insert(new_checkpoint, _active_checkpoint);
		_active_checkpoint = new_checkpoint;
		++_num_checkpoints;
		// Wake up flattener thread
		_sema.up();
		// don't detach old head; this will be done by checkpoint flattener
		if(redundant_memory_verbose_debug)
			Genode::log("Created redundant memory checkpoint");
	}

	void _flatten_previous_snapshots()
	{
		Redundant_checkpoint* reference = _checkpoints.first();
		Redundant_checkpoint* changes = reference->next();
		Genode::size_t num_checkpoints_before = _num_checkpoints;
		while(changes != _active_checkpoint && changes != nullptr)
		{
			if(redundant_memory_verbose_debug)
				Genode::log("Flattening redundant memory snapshots");
			for(Genode::size_t i = 0; i < size; i++)
			{
				//if a byte was changed in the newer snapshot,
				//apply it to the reference
				if(changes->_written_bytes->get(i,1))
				{
					*((Genode::uint8_t*)reference->_addr+i) = *((Genode::uint8_t*)changes->_addr+i);
				}
			}
			//the reference now contains all the changes,
			//the newer snapshot can thus be deleted.
			changes->detach();
			_checkpoints.remove(changes);
			if(!fixed_snapshot_amount)
			{
				_parent_ram.free(Genode::static_cap_cast<Genode::Ram_dataspace>(changes->red_ds_cap));
				Genode::destroy(_alloc, changes);
			}
			--_num_checkpoints;
			changes = reference->next();
		}

		// Move active checkpoint to array position 1 to avoid overwriting
		if(fixed_snapshot_amount)
		{
			Redundant_checkpoint* temp = _allocated_snapshots[1];
			_allocated_snapshots[1] = _allocated_snapshots[num_checkpoints_before-1];
			_allocated_snapshots[num_checkpoints_before-1] = temp;
		}
	}


	class Flattener_thread : public Genode::Thread
	{
	private:
		Designated_redundant_ds_info* _parent;
	public:
		Flattener_thread(Designated_redundant_ds_info* parent) :
			Thread(parent->_env, "Redundant memory snapshot flattener", 16*1024),
			_parent(parent)
		{
		}

		void entry()
		{
			while(true)
			{
				//_sema is for waking up the thread when adding a new snapshot
				_parent->_sema.down();
				//_lock makes sure that the restorer waits until the checkpoints
				//are flattened into one before restoring. Otherwise he would
				//be restoring from an incremental snapshot and thus from
				//uninitialized memory
				_parent->_lock.lock();
				while(_parent->_num_checkpoints > 2)
				{
					_parent->_flatten_previous_snapshots();
				}
				_parent->_lock.unlock();
			}
		}
	} _flattener_thread;

public:
	/**
	 * Constructor
	 */
	Designated_redundant_ds_info(Managed_region_map_info &mrm_info, Genode::Dataspace_capability ds_cap,
			Genode::addr_t addr, Genode::size_t size, Genode::Env& env, Genode::Allocator& alloc, Genode::Region_map& rm,
			Genode::Ram_connection& parent_ram, Genode::Cache_attribute cached)
			:
	Designated_dataspace_info(mrm_info, ds_cap, addr, size, true),
	_sema(0),
	_lock(Genode::Lock::UNLOCKED),
	_parent_ram(parent_ram),
	_cached(cached),
	_env(env),
	_alloc(alloc),
	_rm(rm),
	_num_checkpoints(0),
	_redundant_writing(false),
	_new_checkpoint_pending(false),
	_primary_ds_attached_locally(false),
	_primary_ds_local_addr(0),
	_flattener_thread(this)
	{
	}

	/**
	 * Destructor
	 */
	~Designated_redundant_ds_info()
	{
		_lock.lock();
		for(Redundant_checkpoint* i = _checkpoints.first(); i != nullptr; i = i->next())
		{
			_parent_ram.free(Genode::static_cap_cast<Genode::Ram_dataspace>(i->red_ds_cap));
			Genode::destroy(_alloc, i);
		}
		_lock.unlock();
	}

	bool primary_ds_attached_locally()
	{
		return _primary_ds_attached_locally;
	}

	Genode::addr_t attach_primary_ds_locally()
	{
		if(!_primary_ds_attached_locally)
		{
			_primary_ds_local_addr = _env.rm().attach(cap);
			_primary_ds_attached_locally = true;
		}
		return _primary_ds_local_addr;
	}

	void detach_primary_ds_locally()
	{
		if(_primary_ds_attached_locally)
		{
			_env.rm().detach(_primary_ds_local_addr);
			_primary_ds_local_addr = 0;
			_primary_ds_attached_locally = false;
		}
	}

	Genode::addr_t primary_ds_local_addr()
	{
		return _primary_ds_local_addr;
	}

	bool redundant_writing()
	{
		return _redundant_writing;
	}

	void redundant_writing(bool enable)
	{
		lock();
		if(enable && !_redundant_writing)
		{
			_redundant_writing = true;
			if(_num_checkpoints == 0)
			{
				_allocate_snapshots();
				_create_new_checkpoint();
				_active_checkpoint = _checkpoints.first();
				//first checkpoint is always cumulative
				//since it records writes since the start
				_active_checkpoint->_is_cumulative = true;
				_flattener_thread.start();
			}
			detach();
		}
		else if(!enable && _redundant_writing)
		{
			Genode::warning("Disabling redundant memory serves only debugging purposes and might have unwanted side-effects!");
			_redundant_writing = false;
			attach();
		}
		unlock();
	}

	void trigger_new_checkpoint()
	{
		_new_checkpoint_pending = true;
	}

	void new_checkpoint_if_pending()
	{
		if(_new_checkpoint_pending)
		{
			_create_new_checkpoint();
			_new_checkpoint_pending = false;
		}
	}

	Redundant_checkpoint*  get_active_checkpoint()
	{
		return _active_checkpoint;
	}

	Genode::addr_t get_active_checkpoint_addr()
	{
		return _active_checkpoint->get_address();
	}

	Redundant_checkpoint* get_first_checkpoint()
	{
		return _checkpoints.first();
	}

	void copy_from_latest_checkpoint(void* dst)
	{
		if(_checkpoints.first() == nullptr)
		{
			Genode::error("No snapshot available! Is redundant writing enabled?");
			return;
		}
		_lock.lock();
		Genode::memcpy(dst, (Genode::uint8_t*) _checkpoints.first()->_addr, size);
		_lock.unlock();
	}

	// dst is relative to dataspace start address
	void write_in_active_snapshot(Genode::addr_t dst, void* src, Genode::size_t data_size)
	{
		// Do the actual write
		Genode::memcpy((Genode::uint8_t*)get_active_checkpoint_addr() + dst, src, data_size);
		// Mark written bytes
		_active_checkpoint->_written_bytes->set(dst, data_size);
	}

	void print(Genode::Output &output) const
	{
		Genode::print(output, "Designated_redundant_ds_info, size ", size, ", "
				"recorded writes (if any) listed below, fmt: \"<rel addr>: <content>; [...]\"");
		for(auto i = _checkpoints.first(); i != nullptr; i=i->next())
		{
			Genode::print(output, *i);
		}
	}

};


#endif /* _RTCR_REDUNDANT_MEMORY_DS_INFO_H_ */
