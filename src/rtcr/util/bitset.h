/**
 * Implements a subclass of Bit_array_base WITHOUT
 * exceptions being caused by setting an already
 * set bit.
 */

#ifndef _RTCR_BITSET_H_
#define _RTCR_BITSET_H_

#include <util/bit_array.h>


namespace Rtcr {
	class Bitset;
}

class Rtcr::Bitset : public Genode::Bit_array_base
{
public:
	Bitset(unsigned bits, Genode::addr_t *addr, bool clear) :
		Genode::Bit_array_base(bits, addr, clear)
	{}

	void set(Genode::addr_t const index, Genode::addr_t const width) {
		_set(index, width, false); }

private:

	/* Set method without causing an exception on setting previously set bits */
	void _set(Genode::addr_t index, Genode::addr_t width, bool free)
	{
		_check_range(index, width);

		Genode::addr_t rest, word, mask;
		do {
			word = _word(index);
			mask = _mask(index, width, rest);

			if (free) {
				if ((_words[word] & mask) != mask)
					throw Invalid_clear();
				_words[word] &= ~mask;
			} else {
				_words[word] |= mask;
			}

			index = (_word(index) + 1) * BITS_PER_WORD;
			width = rest;
		} while (rest);
	}

};


#endif /* _RTCR_BIT_ARRAY_H_ */
