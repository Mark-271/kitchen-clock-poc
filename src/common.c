#include <common.h>
#include <stdio.h>

/**
 * Error handling routine.
 *
 * Prints error message and hang in endless loop.
 *
 * @param err Error number
 */
void __attribute__((__noreturn__)) hang(int err)
{
	printf("Error: %d\n", err);
	for (;;)
		;
}
