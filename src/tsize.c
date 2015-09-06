/*
 * tsize.c
 *
 * Terminal size setup.
 *
 * Copyright (c) 2015, Matthew Madison.
 * Distributed under license, see LICENSE for more information.
 */

#include <stdio.h>
#include "termio_raw.h"

int
main (void)
{
	termio_ctx_t tc;

	tc = termio_init(fileno(stdin));
	if (tc != NULL) {
		termio_termsize_setup(tc, NULL);
		termio_finish(tc);
	}
	return 0;
}

