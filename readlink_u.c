/*	readlink - toolbox
	Copyright 2015 libdll.so

	This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>
#include <errno.h>

static void print_usage(const char *name) {
	fprintf(stderr, "Usage: %s [-n|-q] <path> [...]\n", name);
}

int readlink_main(int argc, char **argv) {
	int r = 0;
	int no_newline = 0;
	int quiet = 0;
	int i = 0;
	void move_argv() {
		int j;
		for(j=0; j<i; j++) argv[j + 1] = argv[j];
		argc--;
		argv++;
	}
	while(argv[i]) {
		if(argv[i][0] == '-' && argv[i][1]) {
			const char *o = argv[i] + 1;
			while(*o) switch(*o++) {
				case 'n':
					no_newline = 1;
					move_argv();
					break;
				case 'q':
					quiet = 1;
					move_argv();
					break;
				case 'h':
					print_usage(argv[0]);
					return 0;
				case '-':
					if(*o) {
						if(strcmp(o, "no-newline") == 0) no_newline = 1;
						else if(strcmp(o, "quiet") == 0) quiet = 1;
						else if(strcmp(o, "help") == 0) {
							print_usage(argv[0]);
							return 0;
						} else {
							fprintf(stderr, "%s: Invalid option '%s'\n", argv[0], argv[i]);
							return -1;
						}
						move_argv();
					} else goto options_parse_end;
				default:
					fprintf(stderr, "%s: Invalid option '-%c'\n", argv[0], o[-1]);
					return -1;
			}
		}
		i++;
	}

options_parse_end:
	if(argc < 2) {
		print_usage(argv[0]);
		return -1;
	}
	if(argc > 2 && no_newline) {
		fprintf(stderr, "%s: Warning: 'no-newline' ignored\n", argv[0]);
		no_newline = 0;
	}

	while(*++argv) {
		char buffer[PATH_MAX+1];
		int len = readlink(*argv, buffer, PATH_MAX);
		if(len < 0) {
			if(errno == EINVAL) {
				r = 1;
				continue;
			}
			if(!quiet) perror(*argv);
			return 1;
		}
		buffer[len] = 0;
		if(no_newline) printf("%s", buffer);
		else puts(buffer);
	}

	return r;
}
