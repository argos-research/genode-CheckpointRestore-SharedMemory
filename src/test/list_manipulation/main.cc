/*
 * \brief  Template test
 * \author Denis Huber
 * \date   2016-08-20
 */

/* Genode includes */
#include <base/env.h>
#include <base/log.h>
#include <base/component.h>
#include <util/list.h>
#include <base/heap.h>

using namespace Genode;

struct My_info : public List<My_info>::Element
{
	Dataspace_capability cap;
	addr_t               addr;
	bool                 valid;

	My_info(Dataspace_capability cap, addr_t addr, bool valid)
	:
		cap(cap), addr(addr), valid(valid)
	{ }
};

List<My_info> copy_list(Allocator &alloc, List<My_info> &from)
{
	List<My_info> result;

	My_info *curr = from.first();

	for(; curr; curr = curr->next())
	{
		My_info *info = new (alloc) My_info(
			curr->cap, curr->addr, curr->valid);
		result.insert(info);
	}

	return result;
}

void print_list(List<My_info> &list)
{
	My_info *curr = list.first();

	for(; curr; curr = curr->next())
	{
		log("cap: ", curr->cap.local_name(),
				" addr: ", Hex(curr->addr),
				" valid: ", curr->valid?"true":"false");
	}
}

void delete_elements_alternating(List<My_info> &list, Allocator &alloc)
{
	bool delete_element = false;

	My_info *curr = list.first();
	while(curr)
	{
		My_info *next = curr->next();

		if(delete_element)
		{
			list.remove(curr);
			destroy(alloc, curr);
		}

		delete_element = !delete_element;

		curr = next;
	}
}

size_t Component::stack_size() { return 64*1024; }

void Component::construct(Genode::Env &env)
{
	log("--- list-manupulation started ---");

	Heap heap1 {env.ram(), env.rm()};
	Heap heap2 {env.ram(), env.rm()};

	Ram_dataspace_capability ds1 = env.ram().alloc(4096);
	Ram_dataspace_capability ds2 = env.ram().alloc(4096);
	Ram_dataspace_capability ds3 = env.ram().alloc(4096);

	My_info *info1 = new (heap1) My_info(ds1, 0x1000, true);
	My_info *info2 = new (heap1) My_info(ds2, 0x2000, true);
	My_info *info3 = new (heap1) My_info(ds3, 0x3000, true);

	log("Original list");
	List<My_info> info_list;
	info_list.insert(info1);
	info_list.insert(info2);
	info_list.insert(info3);
	print_list(info_list);

	log("Copied list plus manipulation");
	List<My_info> copied_list = copy_list(heap2, info_list);
	copied_list.first()->valid = false;
	copied_list.first()->next()->valid = false;
	copied_list.first()->next()->next()->valid = false;
	print_list(copied_list);

	log("Delete alternating elements");
	My_info *info4 = new (heap1) My_info(ds1, 0x4000, true);
	My_info *info5 = new (heap1) My_info(ds1, 0x5000, true);
	My_info *info6 = new (heap1) My_info(ds1, 0x6000, true);
	info_list.insert(info4);
	info_list.insert(info5);
	info_list.insert(info6);
	log("\nBefore");
	print_list(info_list);
	delete_elements_alternating(info_list, heap1);
	log("\nAfter");
	print_list(info_list);



	log("--- list-manipulation ended ---");
}
