/*	rmdir - toolbox
	Copyright 2015 libdll.so

	This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

static void usage() {
	fprintf(stderr,"Usage: rmdir"
#if defined _WIN32 && !defined _WIN32_WNT_NATIVE
		".exe"
#endif
		" <directory> [...]\n");
	//return -1;
}

int main(int argc, char *argv[]) {
	//int symbolic = 0;
	int ret;

	if(argc < 2) {
		usage();
		return -1;
	}

	while(argc > 1) {
		argc--;
		argv++;
		ret = rmdir(argv[0]);
		if(ret < 0) {
//#ifdef _WIN32_WCE
//			fprintf(stderr, "rmdir failed for %s, %d\n", argv[0], errno);
//#else
			fprintf(stderr, "rmdir failed for %s, %s\n", argv[0], strerror(errno));
//#endif
			return ret;
		}
	}

	return 0;
}
