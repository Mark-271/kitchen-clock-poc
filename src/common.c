#include <common.h>
#include <stdio.h>

/**
 * Error handling routine.
 *
 * Prints error message and hang in endless loop.
 */
void __attribute__((__noreturn__)) hang(void)
{
	puts("Error: Reboot your board");
	for (;;)
		;
}
