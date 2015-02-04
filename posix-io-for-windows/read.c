/*
 * read.c: read and implementations for Windows.
 *
 * This file has no copyright assigned and is placed in the Public Domain.
 * This file is a part of the mingw-runtime package.
 * No warranty is given; refer to the file DISCLAIMER within the package.
 *
 * Written by Pedro Alves <pedro_alves@portugalmail.pt> Feb 2007
 *
 */

#include <unistd.h>
#include <windows.h>
#include <errno.h>

int _read(int fd, void *buffer, size_t bufsize) {
	unsigned long int r;
	if(!buffer) {
		errno = EFAULT;
		return -1;
	}
	if(bufsize > 0x7fffffff) bufsize = 0x7fffffff;
	if(!ReadFile((void *)fd, buffer, bufsize, &r, NULL)) return -1;
	return (int)r;
}

int read(int fd, void *buffer, size_t bufsize) {
	return _read(fd, buffer, bufsize);
}
