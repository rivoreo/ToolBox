/*	isptrace1allowed - toolbox
	Copyright 2007-2015 PC GO Ld.
	Copyright 2015 libdll.so

	This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#include <sys/types.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

int isptrace1allowed_main(int argc, char **argv) {
	int quiet = 0;
	if(getuid()) {
		fprintf(stderr, "You are not root, test aborted\n");
		return -2;
	}
	if(argc > 1 && (strcmp(argv[1], "-q") == 0 || strcmp(argv[1], "--quiet") == 0)) quiet = 1;

	errno = 0;
	if(ptrace(PT_ATTACH, 1, NULL, 0) < 0) {
		int e = errno;
		if(e) {
			if(e == EPERM) {
				if(!quiet) puts("No");
				return 1;
			}
			fprintf(stderr, "ptrace: %s\n", strerror(e));
			return 2;
		}
	}
	waitpid(1, NULL, 0);
	kill(1, SIGCONT);
	ptrace(PT_DETACH, 1, NULL, 0);
	if(!quiet) puts("Yes");
	return 0;
}
