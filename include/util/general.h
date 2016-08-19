/*
 * \brief  General helper functions
 * \author Denis Huber
 * \date   2016-08-17
 */

namespace Rtcr
{
	void dump_mem(const void *mem, unsigned int size)
	{
		using namespace Genode;

		const char *p = reinterpret_cast<const char*>(mem);

		log("Block: [", Hex((addr_t)mem), ", ", Hex((addr_t)mem + (addr_t)size), ")");
		for(unsigned int i = 0; i < size/16+1; i++)
		{
			log(Hex(i*16, Hex::PREFIX, Hex::NO_PAD),
					"  ", Hex(p[i*16+0],  Hex::OMIT_PREFIX), " ", Hex(p[i*16+1],  Hex::OMIT_PREFIX),
					" ",  Hex(p[i*16+2],  Hex::OMIT_PREFIX), " ", Hex(p[i*16+3],  Hex::OMIT_PREFIX),
					"  ", Hex(p[i*16+4],  Hex::OMIT_PREFIX), " ", Hex(p[i*16+5],  Hex::OMIT_PREFIX),
					" ",  Hex(p[i*16+6],  Hex::OMIT_PREFIX), " ", Hex(p[i*16+7],  Hex::OMIT_PREFIX),
					"  ", Hex(p[i*16+8],  Hex::OMIT_PREFIX), " ", Hex(p[i*16+9],  Hex::OMIT_PREFIX),
					" ",  Hex(p[i*16+10], Hex::OMIT_PREFIX), " ", Hex(p[i*16+11], Hex::OMIT_PREFIX),
					"  ", Hex(p[i*16+12], Hex::OMIT_PREFIX), " ", Hex(p[i*16+13], Hex::OMIT_PREFIX),
					" ",  Hex(p[i*16+14], Hex::OMIT_PREFIX), " ", Hex(p[i*16+15], Hex::OMIT_PREFIX));
		}

	}

	void print_thread_state(Genode::Thread_state &ts)
	{
		using namespace Genode;

		log("  r0:   ", Hex(ts.r0),                                        "  pos: ", Hex((addr_t)&ts.r0                    - (addr_t)&ts, Hex::PREFIX, Hex::PAD));
		log("  r1:   ", Hex(ts.r1),                                        "  pos: ", Hex((addr_t)&ts.r1                    - (addr_t)&ts, Hex::PREFIX, Hex::PAD));
		log("  r2:   ", Hex(ts.r2),                                        "  pos: ", Hex((addr_t)&ts.r2                    - (addr_t)&ts, Hex::PREFIX, Hex::PAD));
		log("  r3:   ", Hex(ts.r3),                                        "  pos: ", Hex((addr_t)&ts.r3                    - (addr_t)&ts, Hex::PREFIX, Hex::PAD));
		log("  r4:   ", Hex(ts.r4),                                        "  pos: ", Hex((addr_t)&ts.r4                    - (addr_t)&ts, Hex::PREFIX, Hex::PAD));
		log("  r5:   ", Hex(ts.r5),                                        "  pos: ", Hex((addr_t)&ts.r5                    - (addr_t)&ts, Hex::PREFIX, Hex::PAD));
		log("  r6:   ", Hex(ts.r6),                                        "  pos: ", Hex((addr_t)&ts.r6                    - (addr_t)&ts, Hex::PREFIX, Hex::PAD));
		log("  r7:   ", Hex(ts.r7),                                        "  pos: ", Hex((addr_t)&ts.r7                    - (addr_t)&ts, Hex::PREFIX, Hex::PAD));
		log("  r8:   ", Hex(ts.r8),                                        "  pos: ", Hex((addr_t)&ts.r8                    - (addr_t)&ts, Hex::PREFIX, Hex::PAD));
		log("  r9:   ", Hex(ts.r9),                                        "  pos: ", Hex((addr_t)&ts.r9                    - (addr_t)&ts, Hex::PREFIX, Hex::PAD));
		log("  r10:  ", Hex(ts.r10),                                       "  pos: ", Hex((addr_t)&ts.r10                   - (addr_t)&ts, Hex::PREFIX, Hex::PAD));
		log("  r11:  ", Hex(ts.r11),                                       "  pos: ", Hex((addr_t)&ts.r11                   - (addr_t)&ts, Hex::PREFIX, Hex::PAD));
		log("  r12:  ", Hex(ts.r12),                                       "  pos: ", Hex((addr_t)&ts.r12                   - (addr_t)&ts, Hex::PREFIX, Hex::PAD));
		log("  sp:   ", Hex(ts.sp),                                        "  pos: ", Hex((addr_t)&ts.sp                    - (addr_t)&ts, Hex::PREFIX, Hex::PAD));
		log("  lr:   ", Hex(ts.lr),                                        "  pos: ", Hex((addr_t)&ts.lr                    - (addr_t)&ts, Hex::PREFIX, Hex::PAD));
		log("  ip:   ", Hex(ts.ip),                                        "  pos: ", Hex((addr_t)&ts.ip                    - (addr_t)&ts, Hex::PREFIX, Hex::PAD));
		log("  cpsr: ", Hex(ts.cpsr),                                      "  pos: ", Hex((addr_t)&ts.cpsr                  - (addr_t)&ts, Hex::PREFIX, Hex::PAD));
		log("  ce:   ", Hex(ts.cpu_exception),                             "  pos: ", Hex((addr_t)&ts.cpu_exception         - (addr_t)&ts, Hex::PREFIX, Hex::PAD));
		log("  upf:  ", ts.unresolved_page_fault?"true ":"false", "     ", "  pos: ", Hex((addr_t)&ts.unresolved_page_fault - (addr_t)&ts, Hex::PREFIX, Hex::PAD));
		log("  exc:  ", ts.exception?"true ":"false",             "     ", "  pos: ", Hex((addr_t)&ts.exception             - (addr_t)&ts, Hex::PREFIX, Hex::PAD));
		log("  kcap: ", Hex(ts.kcap),                                      "  pos: ", Hex((addr_t)&ts.kcap                  - (addr_t)&ts, Hex::PREFIX, Hex::PAD));
		log("  id:   ", Hex(ts.id),                                        "  pos: ", Hex((addr_t)&ts.id                    - (addr_t)&ts, Hex::PREFIX, Hex::PAD));
		log("  utcb: ", Hex(ts.utcb),                                      "  pos: ", Hex((addr_t)&ts.utcb                  - (addr_t)&ts, Hex::PREFIX, Hex::PAD));
		log("  excs: ", Hex(ts.exceptions),                                "  pos: ", Hex((addr_t)&ts.exceptions            - (addr_t)&ts, Hex::PREFIX, Hex::PAD));
		log("  psd:  ", ts.paused?"true ":"false",                "     ", "  pos: ", Hex((addr_t)&ts.paused                - (addr_t)&ts, Hex::PREFIX, Hex::PAD));
		log("  iexc: ", ts.in_exception?"true ":"false",          "     ", "  pos: ", Hex((addr_t)&ts.in_exception          - (addr_t)&ts, Hex::PREFIX, Hex::PAD));
		log("  lock: ", "/",                                  "         ", "  pos: ", Hex((addr_t)&ts.lock                  - (addr_t)&ts, Hex::PREFIX, Hex::PAD));
	}
}
