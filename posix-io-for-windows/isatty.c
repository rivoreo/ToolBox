/*
 * This file has no copyright assigned and is placed in the Public Domain.
 * No warranty is given.
 *
 * Written by PC GO
 *
 */

#include <windows.h>
#include <errno.h>

int isatty(int fd) {
	if(fd == -1) {
		errno = EBADF;
		return 0;
	}
	return GetFileType((void *)fd) == FILE_TYPE_CHAR;
}
