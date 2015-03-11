/*	getopt - toolbox
	Copyright 2015 libdll.so

	This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

int getopt_main(int argc, char **argv) {
	int r = 0;
	int compatible = 0;
	if(getenv("GETOPT_COMPATIBLE")) compatible = 1;
	if(argc < 2) {
		if(compatible) {
			puts(" --");
			return 0;
		}
		return 1;
	}
	const char *options = argv[1];
	argv[1] = argv[0];
	argc--;
	argv++;
	while(1) {
		int c = getopt(argc, argv, options);
		if(c == -1) break;
		if(c == '?') r = 1;
		else {
			printf(" -%c", c);
			if(optarg) printf(" %s", optarg);
		}
	}
	printf(" --");
	while(optind < argc) printf(" %s", argv[optind++]);
	putchar('\n');
	return r;
}
