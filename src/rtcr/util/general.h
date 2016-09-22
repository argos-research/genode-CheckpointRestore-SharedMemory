/*
 * \brief  General helper functions
 * \author Denis Huber
 * \date   2016-08-17
 */

#ifndef _INCLUDE__RTCR_UTIL__GENERAL_H_
#define _INCLUDE__RTCR_UTIL__GENERAL_H_

#include "../ram_session_component.h"
#include "../target_copy.h"

namespace Rtcr
{
	void dump_mem(const void *mem, unsigned int size)
	{
		using namespace Genode;

		const char *p = reinterpret_cast<const char*>(mem);

		log("Block: [", Hex((addr_t)mem), ", ", Hex((addr_t)mem + (addr_t)size), ")");
		for(unsigned int i = 0; i < size/16+1; i++)
		{
			log(Hex(i*16, Hex::PREFIX, Hex::PAD),
					"  ", Hex(p[i*16+0],  Hex::OMIT_PREFIX, Hex::PAD), " ", Hex(p[i*16+1],  Hex::OMIT_PREFIX, Hex::PAD),
					" ",  Hex(p[i*16+2],  Hex::OMIT_PREFIX, Hex::PAD), " ", Hex(p[i*16+3],  Hex::OMIT_PREFIX, Hex::PAD),
					"  ", Hex(p[i*16+4],  Hex::OMIT_PREFIX, Hex::PAD), " ", Hex(p[i*16+5],  Hex::OMIT_PREFIX, Hex::PAD),
					" ",  Hex(p[i*16+6],  Hex::OMIT_PREFIX, Hex::PAD), " ", Hex(p[i*16+7],  Hex::OMIT_PREFIX, Hex::PAD),
					"  ", Hex(p[i*16+8],  Hex::OMIT_PREFIX, Hex::PAD), " ", Hex(p[i*16+9],  Hex::OMIT_PREFIX, Hex::PAD),
					" ",  Hex(p[i*16+10], Hex::OMIT_PREFIX, Hex::PAD), " ", Hex(p[i*16+11], Hex::OMIT_PREFIX, Hex::PAD),
					"  ", Hex(p[i*16+12], Hex::OMIT_PREFIX, Hex::PAD), " ", Hex(p[i*16+13], Hex::OMIT_PREFIX, Hex::PAD),
					" ",  Hex(p[i*16+14], Hex::OMIT_PREFIX, Hex::PAD), " ", Hex(p[i*16+15], Hex::OMIT_PREFIX, Hex::PAD));
		}

	}

	void print_thread_state(Genode::Thread_state &ts, bool brief = false)
	{
		using namespace Genode;

		if(!brief)
		{
			log("  r0:   ", Hex(ts.r0,            Hex::PREFIX, Hex::PAD),      "  pos: ", Hex((addr_t)&ts.r0                    - (addr_t)&ts, Hex::PREFIX, Hex::PAD));
			log("  r1:   ", Hex(ts.r1,            Hex::PREFIX, Hex::PAD),      "  pos: ", Hex((addr_t)&ts.r1                    - (addr_t)&ts, Hex::PREFIX, Hex::PAD));
			log("  r2:   ", Hex(ts.r2,            Hex::PREFIX, Hex::PAD),      "  pos: ", Hex((addr_t)&ts.r2                    - (addr_t)&ts, Hex::PREFIX, Hex::PAD));
			log("  r3:   ", Hex(ts.r3,            Hex::PREFIX, Hex::PAD),      "  pos: ", Hex((addr_t)&ts.r3                    - (addr_t)&ts, Hex::PREFIX, Hex::PAD));
			log("  r4:   ", Hex(ts.r4,            Hex::PREFIX, Hex::PAD),      "  pos: ", Hex((addr_t)&ts.r4                    - (addr_t)&ts, Hex::PREFIX, Hex::PAD));
			log("  r5:   ", Hex(ts.r5,            Hex::PREFIX, Hex::PAD),      "  pos: ", Hex((addr_t)&ts.r5                    - (addr_t)&ts, Hex::PREFIX, Hex::PAD));
			log("  r6:   ", Hex(ts.r6,            Hex::PREFIX, Hex::PAD),      "  pos: ", Hex((addr_t)&ts.r6                    - (addr_t)&ts, Hex::PREFIX, Hex::PAD));
			log("  r7:   ", Hex(ts.r7,            Hex::PREFIX, Hex::PAD),      "  pos: ", Hex((addr_t)&ts.r7                    - (addr_t)&ts, Hex::PREFIX, Hex::PAD));
			log("  r8:   ", Hex(ts.r8,            Hex::PREFIX, Hex::PAD),      "  pos: ", Hex((addr_t)&ts.r8                    - (addr_t)&ts, Hex::PREFIX, Hex::PAD));
			log("  r9:   ", Hex(ts.r9,            Hex::PREFIX, Hex::PAD),      "  pos: ", Hex((addr_t)&ts.r9                    - (addr_t)&ts, Hex::PREFIX, Hex::PAD));
			log("  r10:  ", Hex(ts.r10,           Hex::PREFIX, Hex::PAD),      "  pos: ", Hex((addr_t)&ts.r10                   - (addr_t)&ts, Hex::PREFIX, Hex::PAD));
			log("  r11:  ", Hex(ts.r11,           Hex::PREFIX, Hex::PAD),      "  pos: ", Hex((addr_t)&ts.r11                   - (addr_t)&ts, Hex::PREFIX, Hex::PAD));
			log("  r12:  ", Hex(ts.r12,           Hex::PREFIX, Hex::PAD),      "  pos: ", Hex((addr_t)&ts.r12                   - (addr_t)&ts, Hex::PREFIX, Hex::PAD));
			log("  sp:   ", Hex(ts.sp,            Hex::PREFIX, Hex::PAD),      "  pos: ", Hex((addr_t)&ts.sp                    - (addr_t)&ts, Hex::PREFIX, Hex::PAD));
			log("  lr:   ", Hex(ts.lr,            Hex::PREFIX, Hex::PAD),      "  pos: ", Hex((addr_t)&ts.lr                    - (addr_t)&ts, Hex::PREFIX, Hex::PAD));
			log("  ip:   ", Hex(ts.ip,            Hex::PREFIX, Hex::PAD),      "  pos: ", Hex((addr_t)&ts.ip                    - (addr_t)&ts, Hex::PREFIX, Hex::PAD));
			log("  cpsr: ", Hex(ts.cpsr,          Hex::PREFIX, Hex::PAD),      "  pos: ", Hex((addr_t)&ts.cpsr                  - (addr_t)&ts, Hex::PREFIX, Hex::PAD));
			log("  ce:   ", Hex(ts.cpu_exception, Hex::PREFIX, Hex::PAD),      "  pos: ", Hex((addr_t)&ts.cpu_exception         - (addr_t)&ts, Hex::PREFIX, Hex::PAD));
			log("  upf:  ", ts.unresolved_page_fault?"true ":"false", "     ", "  pos: ", Hex((addr_t)&ts.unresolved_page_fault - (addr_t)&ts, Hex::PREFIX, Hex::PAD));
			log("  exc:  ", ts.exception?"true ":"false",             "     ", "  pos: ", Hex((addr_t)&ts.exception             - (addr_t)&ts, Hex::PREFIX, Hex::PAD));
			log("  kcap: ", Hex(ts.kcap,          Hex::PREFIX, Hex::PAD),      "  pos: ", Hex((addr_t)&ts.kcap                  - (addr_t)&ts, Hex::PREFIX, Hex::PAD));
			log("  id:   ", Hex(ts.id,            Hex::PREFIX, Hex::PAD),      "  pos: ", Hex((addr_t)&ts.id                    - (addr_t)&ts, Hex::PREFIX, Hex::PAD));
			log("  utcb: ", Hex(ts.utcb,          Hex::PREFIX, Hex::PAD),      "  pos: ", Hex((addr_t)&ts.utcb                  - (addr_t)&ts, Hex::PREFIX, Hex::PAD));
			log("  excs: ", Hex(ts.exceptions,    Hex::PREFIX, Hex::PAD),      "  pos: ", Hex((addr_t)&ts.exceptions            - (addr_t)&ts, Hex::PREFIX, Hex::PAD));
			log("  psd:  ", ts.paused?"true ":"false",                "     ", "  pos: ", Hex((addr_t)&ts.paused                - (addr_t)&ts, Hex::PREFIX, Hex::PAD));
			log("  iexc: ", ts.in_exception?"true ":"false",          "     ", "  pos: ", Hex((addr_t)&ts.in_exception          - (addr_t)&ts, Hex::PREFIX, Hex::PAD));
			log("  lock: ", "/",                                  "         ", "  pos: ", Hex((addr_t)&ts.lock                  - (addr_t)&ts, Hex::PREFIX, Hex::PAD));
		}
		else
		{
			log("   r0-r4:  ", Hex(ts.r0,  Hex::PREFIX, Hex::PAD), "  ", Hex(ts.r1,  Hex::PREFIX, Hex::PAD), "  ", Hex(ts.r2,  Hex::PREFIX, Hex::PAD), "  ", Hex(ts.r3, Hex::PREFIX, Hex::PAD),  "  ", Hex(ts.r4, Hex::PREFIX, Hex::PAD));
			log("   r5-r9:  ", Hex(ts.r5,  Hex::PREFIX, Hex::PAD), "  ", Hex(ts.r6,  Hex::PREFIX, Hex::PAD), "  ", Hex(ts.r7,  Hex::PREFIX, Hex::PAD), "  ", Hex(ts.r8, Hex::PREFIX, Hex::PAD),  "  ", Hex(ts.r9, Hex::PREFIX, Hex::PAD));
			log("  r10-r12: ", Hex(ts.r10, Hex::PREFIX, Hex::PAD), "  ", Hex(ts.r11, Hex::PREFIX, Hex::PAD), "  ", Hex(ts.r12, Hex::PREFIX, Hex::PAD));
			log("  ip, sp:  ", Hex(ts.ip,  Hex::PREFIX, Hex::PAD), "  ", Hex(ts.sp,  Hex::PREFIX, Hex::PAD));
		}
	}

	void print_attached_region_info_list(Genode::List<Attached_region_info> &list)
	{
		using namespace Genode;

		//log("Attached_region_info list");


		for(Attached_region_info *curr_ar = list.first(); curr_ar; curr_ar = curr_ar->next())
		{
			log("  Dataspace ", curr_ar->ds_cap,
					" occupying [", Hex(curr_ar->addr),
					", ", Hex(curr_ar->addr + curr_ar->size), ")",
					curr_ar->executable ? " exec": "");
		}
	}

	void print_managed_region_info_list(Genode::List<Managed_region_info> &list, bool brief = false)
	{
		using namespace Genode;

		//log("Managed_region_info list");


		for(Managed_region_info *curr_mr = list.first(); curr_mr; curr_mr = curr_mr->next())
		{
			unsigned int buffer_size = 100;
			char cloned_dataspaces_string[buffer_size] = "";

			if(brief)
			{
				unsigned int num_ds = 0;
				for(Attachable_dataspace_info *curr_ad = curr_mr->ad_infos.first(); curr_ad; curr_ad = curr_ad->next())
				{
					num_ds++;
				}

				snprintf(cloned_dataspaces_string, buffer_size, "%d dataspace(s)", num_ds);
			}
			else
			{
				unsigned int used = 0;
				for(Attachable_dataspace_info *curr_ad = curr_mr->ad_infos.first(); curr_ad; curr_ad = curr_ad->next())
				{
					if(used + 10 > buffer_size)
					{
						used = used + snprintf(cloned_dataspaces_string + used, buffer_size - used, " ...");
						break;
					}
					else if(used == 0)
					{
						used = used + snprintf(cloned_dataspaces_string + used, buffer_size - used, "%ld%s", curr_ad->ds_cap.local_name(), curr_ad->attached?"a":"d");
					}
					else
					{
						used = used + snprintf(cloned_dataspaces_string + used, buffer_size - used, " %ld%s", curr_ad->ds_cap.local_name(), curr_ad->attached?"a":"d");
					}
				}
			}

			size_t mds_size = Dataspace_client{curr_mr->mds_cap}.size();

			log("  Managed dataspace ", curr_mr->mds_cap,
					" (", const_cast<const char*>(cloned_dataspaces_string), ")",
					" size ", Hex(mds_size, Hex::PREFIX, Hex::PAD),
					", Region_map ", curr_mr->rm_cap);
		}
	}

	void print_copied_region_info_list(Genode::List<Copied_region_info> &list, bool brief = false)
	{
		using namespace Genode;

		for(Copied_region_info *curr_cr = list.first(); curr_cr; curr_cr = curr_cr->next())
		{
			unsigned int buffer_size = 100;
			char dataspaces_string[buffer_size] = "";

			if(brief)
			{
				unsigned int num_ds = 0;
				for(Copied_dataspace_info *cd_info = curr_cr->cloned_dataspaces.first(); cd_info; cd_info = cd_info->next())
				{
					num_ds++;
				}

				snprintf(dataspaces_string, buffer_size, "%d dataspace(s)", num_ds);
			}
			else
			{
				unsigned int used = 0;
				for(Copied_dataspace_info *cd_info = curr_cr->cloned_dataspaces.first(); cd_info; cd_info = cd_info->next())
				{
					if(used + 9 > buffer_size)
					{
						used = used + snprintf(dataspaces_string + used, buffer_size - used, " ...");
						break;
					}
					else if(used == 0)
					{
						used = used + snprintf(dataspaces_string + used, buffer_size - used, "%ld", cd_info->ds_cap.local_name());
					}
					else
					{
						used = used + snprintf(dataspaces_string + used, buffer_size - used, " %ld", cd_info->ds_cap.local_name());
					}
				}
			}

			log("  Copied original dataspace ", curr_cr->original_ds_cap,
					" into (", const_cast<const char*>(dataspaces_string), ")",
					" occupying [", Hex(curr_cr->addr),
					", ", Hex(curr_cr->addr + curr_cr->size), ")");
		}
	}
}

#endif /* _INCLUDE__RTCR_UTIL__GENERAL_H_ */
