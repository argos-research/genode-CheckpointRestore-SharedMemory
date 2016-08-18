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

		log("Block: [", mem, ", ", mem + size, ")");
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
}
