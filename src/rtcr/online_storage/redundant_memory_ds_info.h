#ifndef _RTCR_REDUNDANT_MEMORY_DS_INFO_H_
#define _RTCR_REDUNDANT_MEMORY_DS_INFO_H_

#include "ram_dataspace_info.h"


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
		bool _attached;
		Genode::Dataspace_capability const  _red_ds_cap;
		Genode::addr_t _addr;
		bool _is_cumulative;
		Genode::size_t _size;
		Genode::Region_map& _rm;
	public:
		// The actual ds used for redundant writing

		/* If the snapshot is not cumulative, it is not a complete
		 * memory snapshot, i.e. some blocks refer to the previous
		 * snapshot.
		 */
		Redundant_checkpoint(Genode::Dataspace_capability const  ds_cap, Genode::size_t size,
				Genode::Region_map& rm)
		:
			_attached(false), _red_ds_cap(ds_cap), _addr(0),
			_is_cumulative(false), _size(size),
			_rm(rm)
		{}

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
	_rm(rm)
	{
		//Create first snapshot
		Genode::Dataspace_capability const red_ds = _parent_ram.alloc(size,_cached);
		create_new_checkpoint();
		_current_checkpoint = _checkpoints.first();
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
		Redundant_checkpoint* new_checkpoint = new (_alloc) Redundant_checkpoint(ds, size, _rm);
		new_checkpoint->attach();
		_checkpoints.insert(new_checkpoint);
		_current_checkpoint = new_checkpoint;
		// don't detach old head; this will be done by checkpoint flattener
	}
};


#endif /* _RTCR_REDUNDANT_MEMORY_DS_INFO_H_ */
