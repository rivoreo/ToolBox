/*	unlink - toolbox
	Copyright 2007-2015 PC GO Ld.

	This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

void welcome() {
	puts("unlink - toolbox\nCopyright 2007-2015 PC GO Ld.\nRemove existing file.\n");
	fprintf(stderr, "Usage: unlink"
#if defined _WIN32 && !defined _WIN32_WNT_NATIVE
		".exe"
#endif
		" <target>\n");
}

int main(int argc, char *argv[]) {
	if(argc == 1 || strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
		welcome();
		return -1;
	}
	if(argc > 1 && strcmp(argv[1], "--") == 0) {
		if(argc < 3) {
			fprintf(stderr, "%s: No target specified\n", argv[0]);
			return 1;
		}
		argv[1] = argv[0];
		argc--;
		argv++;
	}
/*
	if (optind < 1 || optind >= argc) {
		fprintf(stderr, "Unknown arguments\n");
		return 1;
	}*/
	if(argc > 2) {
		fprintf(stderr, "%s: Too many arguments\n", argv[0]);
		return 1;
	}
#if defined _WIN32 && !defined _WINDOWNSNT_NATIVE
	if(access(argv[1], F_OK) == 0 && access(argv[1], W_OK) != 0) chmod(argv[1], 666);
#endif
	if(unlink(argv[1]) < 0) {
		fprintf(stderr, "unlink failed for %s, %s\n", argv[1], strerror(errno));
		return 1;
	}
	return 0;
}
