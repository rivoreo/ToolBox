/*
 * open.c: open implementations for Windows.
 *
 * This file has no copyright assigned and is placed in the Public Domain.
 * This file is a part of the mingw-runtime package.
 * No warranty is given; refer to the file DISCLAIMER within the package.
 *
 * Written by Pedro Alves <pedro_alves@portugalmail.pt> Feb 2007
 *
 */

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <windows.h>
#include <limits.h>
#include <errno.h>

static int vopen(const char *path, int oflag, va_list ap) {
	unsigned long int fileaccess;
	unsigned long int fileshare;
	unsigned long int filecreate;
	unsigned long int fileattrib = 0;
	void *hnd;

	if(!path) {
		errno = EFAULT;
		return -1;
	}

	wchar_t wpath[PATH_MAX];
	size_t path_len = strlen(path) + 1;
	if(path_len == 1) {
		errno = ENOENT;
		return -1;
	}
	if(path_len > PATH_MAX) {
		errno = ENAMETOOLONG;
		return -1;
	}
	if(mbstowcs(wpath, path, path_len) == (size_t)-1) {
		//errno = ENOENT;
		return -1;
	}

	switch(oflag & (O_RDONLY | O_WRONLY | O_RDWR)) {
		case O_RDONLY:
			fileaccess = GENERIC_READ;
			break;
		case O_WRONLY:
			fileaccess = GENERIC_WRITE;
			break;
		case O_RDWR:
			fileaccess = GENERIC_READ | GENERIC_WRITE;
			break;
		default:
			errno = EINVAL;
			return -1;
	}

	switch (oflag & (O_CREAT | O_EXCL | O_TRUNC)) {
		case 0:
		case O_EXCL:		/* ignore EXCL w/o CREAT */
			filecreate = OPEN_EXISTING;
			break;
		case O_CREAT:
			filecreate = OPEN_ALWAYS;
			break;
		case O_CREAT | O_EXCL:
		case O_CREAT | O_TRUNC | O_EXCL:
			filecreate = CREATE_NEW;
			break;

		case O_TRUNC:
		case O_TRUNC | O_EXCL:	/* ignore EXCL w/o CREAT */
			filecreate = TRUNCATE_EXISTING;
			break;
		case O_CREAT | O_TRUNC:
			filecreate = CREATE_ALWAYS;
			break;
		default:
			/* this can't happen ... all cases are covered */
			return -1;
	}

	fileshare = FILE_SHARE_READ;

	if(oflag & O_CREAT) {
		mode_t pmode = va_arg(ap, int);
		if((pmode & S_IWRITE) != S_IWRITE) fileattrib |= FILE_ATTRIBUTE_READONLY;
	}
	if(!fileattrib) fileattrib = FILE_ATTRIBUTE_NORMAL;

	hnd = CreateFileW(wpath, fileaccess, fileshare, NULL, filecreate, fileattrib, NULL);
	if(hnd == INVALID_HANDLE_VALUE) return -1;
	if(oflag & O_APPEND) SetFilePointer(hnd, 0, NULL, FILE_END);
	return (int)hnd;
}

int _open(const char *path, int oflag, ...) {
	va_list ap;
	int ret;
	va_start(ap, oflag);
	ret = vopen(path, oflag, ap);
	va_end(ap);
	return ret;
}

int open(const char *path, int oflag, ...) {
	va_list ap;
	int ret;
	va_start(ap, oflag);
	ret = vopen(path, oflag, ap);
	va_end(ap);
	return ret;
}
