/*
 * This file has no copyright assigned and is placed in the Public Domain.
 * No warranty is given.
 *
 * Written by PC GO
 *
 */

#include <unistd.h>
#include <fcntl.h>

int creat(const char *file, mode_t mode) {
	return open(file, O_CREAT | O_WRONLY | O_TRUNC, mode);
}
