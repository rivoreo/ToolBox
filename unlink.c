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
	int i;
	if(argc == 1 || !strcmp(argv[1], "-h") || !strcmp(argv[1], "--help")) {
		welcome();
		return -1;
	}
	if (optind < 1 || optind >= argc) {
		fprintf(stderr, "Unknown arguments\n");
		return 1;
	}
	for(i = optind; i < argc; i++) {
#if defined _WIN32 && !defined _WIN32_WNT_NATIVE
		if(access(argv[i], F_OK) == 0 && access(argv[i], W_OK) != 0) chmod(argv[i], 666);
#endif
		int ret = unlink(argv[i]);
		if(ret < 0) {
			fprintf(stderr, "unlink failed for %s, %s\n", argv[i], strerror(errno));
			return 1;
		}
	}
	return 0;
}
