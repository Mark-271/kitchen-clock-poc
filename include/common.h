#ifndef COMMON_H
#define COMMON_H

#include <string.h>

#define BIT(n)		(1 << n)
#define ARRAY_SIZE(a)	(sizeof(a) / sizeof(a[0]))

/**
 *  Reverse the given null-terminated string in place.
 *
 *  Swap the values in the two given variables
 *  Fails when a and b refer to same memory location
 *  This works because of three basic properties of xor:
 *  x ^ 0 = x, x ^ x = 0 and x ^ y = y ^ x for all values x and y
 *
 *  @param input should be an array, whose contents are initialized to
 *  the given string constant.
 */
static void inplace_reverse(char *str)
{
	if (str) {
		char *end = str + strlen(str) - 1;

#define XOR_SWAP(a,b) do\
		{\
			a ^= b;\
			b ^= a;\
			a ^= b;\
		} while (0)

		/* Walk inwards from both ends of the string,
		 * swapping until we get to the middle
		 */
		while (str < end) {
			XOR_SWAP(*str, *end);
			str++;
			end--;
		}
#undef XOR_SWAP
	}
}

#endif /* COMMON_H */
