/*	reboot - toolbox
	Copyright 2015 libdll.so

	This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#include <unistd.h>
#include <sys/reboot.h>
#include <string.h>
#include <stdio.h>

int reboot_main(int argc, char **argv) {
	int flags = RB_AUTOBOOT;
	if(argc < 2 || strcmp(argv[1], "-n")) sync();
#ifdef RB_NOSYNC
	else flags |= RB_NOSYNC;
#endif
	if(reboot(flags) < 0) {
		perror("reboot");
		return 1;
	}

	// Never reached?
	return 0;
}

