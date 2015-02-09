/*	PE header item edit tool
	toolbox
	Copyright 2007-2015 PC GO Ld.
	Copyright 2015 libdll.so

	This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

/*
#define _BATCH_MODIFY
#define main modexeb_main
#include "modexe_u.c"
*/

#include <stdio.h>
#include <assert.h>

extern int modexe_main(int, char **);

int modexeb_main(int argc, char **argv) {
	if(argc < 4) {
		fprintf(stderr, "Usage: %s <item> <new value> <file> [<file>] [...]\n", argv[0]);
		return -1;
	}

	int r = 0;
	char **files = argv + 3;
	while(*files) {
		char *new_argv[] = { argv[0], *files, argv[1], argv[2], NULL };
		int sr = modexe_main(4, new_argv);
		assert(sr >= 0 && sr != 1);
		printf("%s [%s]\n", *files++, sr == 0 ? "OK" : "Fail");
		if(sr) r++;
	}
	return r;
}
