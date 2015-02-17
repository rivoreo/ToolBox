/*	runcon - toolbox
	Copyright 2015 libdll.so

	This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <selinux/selinux.h>

int main(int argc, char **argv) {
	int rc;

	if (argc < 3) {
		fprintf(stderr, "Usage:  %s <context> <program> [<args...>]\n", argv[0]);
		//exit(1);
		return -1;
	}

	rc = setexeccon(argv[1]);
	if (rc < 0) {
		fprintf(stderr, "Could not set context to %s: %s\n", argv[1], strerror(errno));
		//exit(2);
		return 127;
	}

	argv += 2;
	argc -= 2;
	execvp(argv[0], argv);
	fprintf(stderr, "Could not exec %s: %s\n", argv[0], strerror(errno));
	return 3;
}
