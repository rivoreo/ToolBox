/*	setpgrp - toolbox
	Copyright 2015-2016 Rivoreo

	This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/param.h>

#if defined BSD || defined __INTERIX
#define setpgrp() setpgid(0,0)
#endif

int setpgrp_main(int argc, char **argv) {
	if(argc < 2) {
		fprintf(stderr, "Usage: %s <command> [<argument>] [...]\n", argv[0]);
		return -1;
	}
	if(setpgrp() < 0) {
		perror("setpgrp");
		return 1;
	}
	argv++;
	execvp(argv[0], argv);
	perror(argv[0]);
	return 1;
}	
