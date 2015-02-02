/*	insmod - toolbox
	Copyright 2007-2015 PC GO Ld.

	This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <malloc.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

extern int init_module(void *, unsigned long, const char *);

static void *read_file(const char *filename, ssize_t *_size) {
	int ret, fd;
	struct stat sb;
	ssize_t size;
	void *buffer = NULL;

	/* open the file */
	fd = open(filename, O_RDONLY);
	if(fd == -1) return NULL;

	/* find out how big it is */
	if(fstat(fd, &sb) < 0) goto bail;
	size = sb.st_size;

	/* allocate memory for it to be read into */
	buffer = malloc(size);
	if(!buffer) goto bail;

	/* slurp it into our buffer */
	ret = read(fd, buffer, size);
	if(ret != size) goto bail;

	/* let the caller know how big it is */
	*_size = size;

bail:
	close(fd);
	return buffer;
}

#ifndef min
#define min(x,y) ((x) < (y) ? (x) : (y))
#endif
int main(int argc, char **argv) {
	void *file;
	ssize_t size = 0;
	char opts[1024];
	int r;

	/* make sure we've got an argument */
	if (argc < 2) {
		fprintf(stderr, "Usage: %s <module file> [<module options>]\n", argv[0]);
		return -1;
	}

	/* read the file into memory */
	file = read_file(argv[1], &size);
	if(!file) {
		int e = errno;
		if(!e) e = -1;
		fprintf(stderr, "%s: can't open '%s' (%s)\n", argv[0], argv[1], strerror(e));
		return -1;
	}

	opts[0] = 0;
	if(argc > 2) {
		int i, len;
		char *end = opts + sizeof opts - 1;
		char *ptr = opts;

		for(i = 2; i < argc && ptr < end; i++) {
			len = min(strlen(argv[i]), end - ptr);
			memcpy(ptr, argv[i], len);
			ptr += len;
			*ptr++ = ' ';
		}
		ptr[1] = 0;
	}

	/* pass it to the kernel */
	r = init_module(file, size, opts);
	if(r) fprintf(stderr, "%s: init_module '%s' failed (%s)\n", argv[0], argv[1], strerror(errno));

	/* free the file buffer */
	free(file);

	return r;
}
