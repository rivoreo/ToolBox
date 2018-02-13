/*
 * This file has no copyright assigned and is placed in the Public Domain.
 * No warranty is given.
 *
 * Written by Rivoreo
 *
 */

#include <stdio.h>
#include <errno.h>
#include <io.h>

/* XXX: This implementation is currently broken; programs that using fdopen
 * and linking to this library will fail due to the following unresolvable
 * symbols. However this error is still better than allowing link to a wrong
 * version of fdopen in msvcrt.dll.
 */
extern FILE *_getstream(void);
extern int _alloc_osfhnd(void);
extern int _set_osfhnd(int, intptr_t);

FILE *fdopen(int fd, const char *mode) {
#if 0
	extern int __p__commode(void);
	int stream_flag = *__p__commode();
#else
	int stream_flag = 0;
#endif
	if(fd == -1) {
		errno = EBADF;
		return NULL;
	}
	while(*mode == ' ') mode++;
	switch(*mode) {
		case 'r':
			stream_flag |= _IOREAD;
			break;
		case 'w':
			stream_flag |= _IOWRT;
			break;
		case 'a':
			stream_flag |= _IOWRT;
			lseek(fd, 0, SEEK_END);
			break;
		default:
			errno = EINVAL;
			return NULL;
	}
	while(*++mode) switch(*mode) {
		case ' ':
			break;
		case '+':
			//if(stream_flag & _IORW) goto mode_done;
			stream_flag &= _IOREAD | _IOWRT;
			stream_flag |= _IORW;
			break;
		case 'b':
			// TODO
			break;
		default:
			errno = EINVAL;
			return NULL;
	}
//mode_done:

	FILE *r = _getstream();
	if(!r) {
		errno = EMFILE;
		return NULL;
	}
	r->_file = _alloc_osfhnd();
	if(r->_file == -1) {
		errno = EMFILE;
		return NULL;
	}
	_set_osfhnd(r->_file, (intptr_t)fd);
	// TODO: set flags for r->_file, see "msvcrt/open.c"
	r->_flag = stream_flag;

	return r;
}
