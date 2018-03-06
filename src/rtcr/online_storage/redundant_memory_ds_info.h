#ifndef _RTCR_REDUNDANT_MEMORY_DS_INFO_H_
#define _RTCR_REDUNDANT_MEMORY_DS_INFO_H_

#include "ram_dataspace_info.h"

#include <util/bit_array.h>


namespace Rtcr {
	struct Designated_redundant_ds_info;
}


struct Rtcr::Designated_redundant_ds_info: public Rtcr::Designated_dataspace_info
{

	/**
	 * Struct representing one checkpoint.
	 * The list tail is the current checkpoint that is used for redundant writing.
	 */
	struct Redundant_checkpoint : public Genode::List<Redundant_checkpoint>::Element
	{
	private:
		friend class Designated_redundant_ds_info;
		bool _attached;
		Genode::Dataspace_capability const  _red_ds_cap;
		Genode::addr_t _addr;
		bool _is_cumulative;
		Genode::size_t _size;
		Genode::Region_map& _rm;
		Genode::Allocator& _alloc;
		//marks the bytes that have been modified in this snapshot
		Genode::Bit_array_base* _written_bytes;
		Genode::addr_t* _bitset_array;
		static const Genode::size_t BITSET_UNIT_BITSIZE = sizeof(*_bitset_array)*8;
	public:
		/* If the snapshot is not cumulative, it is not a complete
		 * memory snapshot, i.e. some blocks refer to the previous
		 * snapshot.
		 */
		Redundant_checkpoint(Genode::Dataspace_capability const  ds_cap, Genode::size_t size,
				Genode::Allocator& alloc, Genode::Region_map& rm)
		:
			_attached(false), _red_ds_cap(ds_cap), _addr(0),
			_is_cumulative(false), _size(size),	_rm(rm), _alloc(alloc)
		{
			_bitset_array = new(_alloc) Genode::addr_t[_size/BITSET_UNIT_BITSIZE];
			_written_bytes = new(_alloc) Genode::Bit_array_base((_size + BITSET_UNIT_BITSIZE - 1)
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
				return 0;
			}
			_addr = _rm.attach(_red_ds_cap);
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
	};

private:

	/* Head and tail of redundant checkpoint list.
	 * The tail is used for redundant writing.
	 * The head is needed for restoring or
	 * flattening checkpoints.
	 */
	Genode::List<Redundant_checkpoint> _checkpoints;
	Redundant_checkpoint* _current_checkpoint;

	Genode::Ram_connection& _parent_ram;
	Genode::Cache_attribute _cached;

	Genode::Allocator& _alloc;

	Genode::Region_map& _rm;

	Genode::size_t _num_checkpoints;

public:
	/**
	 * Constructor
	 */
	Designated_redundant_ds_info(Managed_region_map_info &mrm_info, Genode::Dataspace_capability ds_cap,
			Genode::addr_t addr, Genode::size_t size, Genode::Allocator& alloc, Genode::Region_map& rm,
			Genode::Ram_connection& parent_ram, Genode::Cache_attribute cached)
			:
	Designated_dataspace_info(mrm_info, ds_cap, addr, size),
	_parent_ram(parent_ram),
	_cached(cached),
	_alloc(alloc),
	_rm(rm),
	_num_checkpoints(0)
	{
		create_new_checkpoint();
		_current_checkpoint = _checkpoints.first();
		//first checkpoint is always cumulative
		//since it records writes since the start
		_current_checkpoint->_is_cumulative = true;
	}

	Redundant_checkpoint*  get_current_checkpoint()
	{
		return _current_checkpoint;
	}

	Genode::addr_t get_current_checkpoint_addr()
	{
		return _current_checkpoint->get_address();
	}

	Redundant_checkpoint* get_first_checkpoint()
	{
		return _checkpoints.first();
	}

	void create_new_checkpoint()
	{
		Genode::Dataspace_capability const ds = _parent_ram.alloc(size,_cached);
		Redundant_checkpoint* new_checkpoint = new (_alloc) Redundant_checkpoint(ds, size, _alloc, _rm);
		new_checkpoint->attach();
		_checkpoints.insert(new_checkpoint, _current_checkpoint);
		_current_checkpoint = new_checkpoint;
		++_num_checkpoints;
		// don't detach old head; this will be done by checkpoint flattener
	}

	// dst is relative to dataspace start address
	void write_in_current_snapshot(Genode::addr_t dst, void* src, Genode::size_t data_size)
	{
		// Do the actual write
		Genode::memcpy((Genode::uint8_t*)get_current_checkpoint_addr() + dst, src, data_size);
		// Mark written bytes
		_current_checkpoint->_written_bytes->set(dst, data_size);
	}

	void flatten_previous_snapshots()
	{
		Redundant_checkpoint* reference = _checkpoints.first();
		Redundant_checkpoint* changes = reference->next();
		while(changes != _current_checkpoint && changes != nullptr)
		{
			PINF("Flattening");
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
			_parent_ram.free(Genode::static_cap_cast<Genode::Ram_dataspace>(changes->_red_ds_cap));
			Genode::destroy(_alloc, changes);
			--_num_checkpoints;
			changes = reference->next();
		}
	}
};


#endif /* _RTCR_REDUNDANT_MEMORY_DS_INFO_H_ */
