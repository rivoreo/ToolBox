/*
 * This file has no copyright assigned and is placed in the Public Domain.
 * No warranty is given.
 *
 * Written by PC GO
 *
 */

#include <stdio.h>
#include <io.h>

#ifdef fileno
#error "fileno macro defined"
#endif

int fileno(FILE *f) {
	return (int)_get_osfhandle(f->_file);
}
