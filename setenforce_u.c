/*	setenforce - toolbox
	Copyright 2015 libdll.so

	This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#include <selinux/selinux.h>

static void usage(const char *progname) {
	fprintf(stderr, "Usage: %s { Enforcing | Permissive | 1 | 0 }\n", progname);
}

int setenforce_main(int argc, char **argv)
{
	int rc = 0;
	if(argc != 2) {
		usage(argv[0]);
		return -1;
	}

	if (is_selinux_enabled() <= 0) {
		fprintf(stderr, "%s: SELinux is disabled\n", argv[0]);
		return 1;
	}
	if (strlen(argv[1]) == 1 && (argv[1][0] == '0' || argv[1][0] == '1')) {
		rc = security_setenforce(atoi(argv[1]));
	} else {
		if(strcasecmp(argv[1], "enforcing") == 0) {
			rc = security_setenforce(1);
		} else if(strcasecmp(argv[1], "permissive") == 0) {
			rc = security_setenforce(0);
		} else {
			usage(argv[0]);
			return -1;
		}
	}
	if (rc < 0) {
		fprintf(stderr, "%s: Could not set enforcing status: %s\n",
			argv[0], strerror(errno));
		return 2;
	}
	return 0;
}
