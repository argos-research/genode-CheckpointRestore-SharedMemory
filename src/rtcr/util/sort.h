/*
 * \brief  Sort implementation
 * \author de.wikibooks.org
 * \date   2016-11-28
 *
 * From https://de.wikibooks.org/wiki/Algorithmen_und_Datenstrukturen_in_C/_Mergesort
 */

#ifndef _RTCR_SORT_H_
#define _RTCR_SORT_H_

/* Genode includes */
#include <base/stdint.h>

namespace Rtcr {
	void merge_sort(Genode::size_t array[], Genode::size_t size);
}

void Rtcr::merge_sort(Genode::size_t array[], Genode::size_t size)
{
	if(size > 1)
	{
		Genode::size_t half1[size/2];
		Genode::size_t half2[(size + 1)/2];
		Genode::size_t i;
		for(i = 0; i < size/2; ++i)
			half1[i] = array[i];
		for(i = size/2; i < size; ++i)
			half2[i - size/2] = array[i];

		merge_sort(half1,size/2);
		merge_sort(half2,(size + 1)/2);

		Genode::size_t *pos1 = &half1[0];
		Genode::size_t *pos2 = &half2[0];
		for(i = 0; i < size; ++i)
		{
			if(*pos1 <= *pos2)
			{
				array[i] = *pos1;
				if (pos1 != &half2[(size+1)/2 - 1])
				{
					if(pos1 == &half1[size/2 - 1])
					{
						pos1 = &half2[(size+1)/2 - 1];
					}
					else
					{
						++pos1;
					}
				}
			}
			else
			{
				array[i] = *pos2;
				if(pos2 == &half2[(size + 1)/2 - 1])
				{
					pos2 = &half1[size/2 - 1];
				}
				else
				{
					++pos2;
				}
			}
		}
	}
}

#endif /* _RTCR_SORT_H_ */
