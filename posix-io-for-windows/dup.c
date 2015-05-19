/*
 * This file has no copyright assigned and is placed in the Public Domain.
 * No warranty is given.
 *
 * Written by PC GO
 *
 */

#include <windows.h>

int dup(int oldfd) {
	void *r;
	int ok = DuplicateHandle(GetCurrentProcess(), (void *)oldfd, GetCurrentProcess(), &r, 0, 1, DUPLICATE_SAME_ACCESS);
	return ok ? (int)r : -1;
}
