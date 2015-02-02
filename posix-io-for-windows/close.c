/*
 * close.c: close implementation for Windows.
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

int _close(int fd) {
	return CloseHandle((void *)fd) ? 0 : -1;
}

int close(int fd) {
	return _close(fd);
}
