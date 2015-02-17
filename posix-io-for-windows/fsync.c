/*
 * This file has no copyright assigned and is placed in the Public Domain.
 * No warranty is given.
 *
 * Written by PC GO
 *
 */

#include <windows.h>

int fsync(int fd) {
	return FlushFileBuffers((void *)fd) ? 0 : -1;
}
