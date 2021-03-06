/*	mv - toolbox
	Copyright 2007-2015 PC GO Ld.
	Copyright 2015 libdll.so

	This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/types.h>

#if defined _WIN32 && !defined _WIN32_WNT_NATIVE
#define lstat stat
#endif

int mv_main(int argc, char *argv[]) {
	char *dest;
	struct stat st;
	int i;

	if (argc < 3) {
		fprintf(stderr,"Usage: %s <source...> <destination>\n", argv[0]);
		return -1;
	}

	/* check if destination exists */
	dest = argv[argc - 1];
	size_t dest_len = strlen(dest);
	if(stat(dest, &st) < 0) {
		/* an error, unless the destination was missing */
		if(errno != ENOENT) {
			fprintf(stderr, "%s: failed on %s - %s\n", argv[0], dest, strerror(errno));
			return -1;
		} else if(argc > 3) {
			fprintf(stderr, "%s: %s: %s\n", argv[0], dest, strerror(ENOTDIR));
			return 1;
		}
		st.st_mode = 0;
	}
	//printf("%s is a dir? %s\n", dest, S_ISDIR(st.st_mode)? "true" : "false");
	if(argc > 3 && !S_ISDIR(st.st_mode)) {
		fprintf(stderr, "%s: %s: %s\n", argv[0], dest, strerror(ENOTDIR));
		return 1;
	}

	for (i = 1; i < argc - 1; i++) {
		const char *source = argv[i];
		char fullDest[PATH_MAX + 1 + PATH_MAX + 1];
		/* assume we build "dest/source", and let rename() fail on pathsize */
		if(dest_len + 1 + strlen(source) + 1 > sizeof(fullDest)) {
			fprintf(stderr, "%s: path too long\n", argv[0]);
			return 2;
		}
		strcpy(fullDest, dest);

		/* if destination is a directory, concat the source file name */
		if(S_ISDIR(st.st_mode)) {
			const char *fileName = strrchr(source, '/');
			if(fullDest[strlen(fullDest)-1] != '/') {
				strcat(fullDest, "/");
			}
			strcat(fullDest, fileName ? fileName + 1 : source);
		}

		if(lstat(source, &st) < 0) {
			perror(source);
			return 3;
		}

		if(strcmp(source, fullDest) == 0) {
			fprintf(stderr, "%s: %s: source and destination are same\n", argv[0], source);
			return 4;
		}

		/* attempt to move it */
		if(rename(source, fullDest) < 0) {
			//perror("mv");
			perror(argv[0]);
			return 4;
		}
	}

	return 0;
}
