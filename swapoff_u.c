/*	swapoff - toolbox
	Copyright 2015 libdll.so

	This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#include <stdio.h>
#include <unistd.h>
//#include <asm/page.h>
#include <sys/swap.h>
#include <string.h>
#include <errno.h>

int swapoff_main(int argc, char **argv) {
	if (argc != 2) {
		fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
		return -EINVAL;
	}

	if(swapoff(argv[1]) < 0) {
		int e = errno;
		fprintf(stderr, "swapoff failed for %s, %s\n", argv[1], strerror(e));
		return -e;
	}

	return 0;
}
