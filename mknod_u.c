/*	mknod - toolbox
	Copyright 2015-2020 Rivoreo

	This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#include <sys/types.h>
#include <sys/stat.h>
#include "str2mode.h"
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#ifndef makedev
#ifdef __GLIBC__
#include <sys/sysmacros.h>
#else
#include <sys/mkdev.h>
#endif
#ifndef makedev
#define makedev mkdev
#endif
#endif

static void print_usage(const char *name) {
	fprintf(stderr, "Usage: %s [{-m|--mode} <mode>] <name> <type> [<major> <minor>]\n", name);
}

int mknod_main(int argc, char **argv) {
	int i = 1;
	mode_t mode = 0777;
	int set_mode(const char *option) {
		const char *m = argv[i + 1];
		if(!m) {
			fprintf(stderr, "%s: Option '%s' requires an argument\n", argv[0], option);
			return -1;
		}
		mode = STR2MODE(m);
		if(mode == (mode_t)-1) {
			fprintf(stderr, "%s: Bad mode\n", argv[0]);
			return -1;
		}
		umask(0);
		int j;
		argc -= 2;
		for(j=i; j<argc+1; j++) argv[j] = argv[j + 2];
		return 0;
	}
first_loop:
	while(i < argc) {
		if(argv[i][0] == '-') {
			const char *o = argv[i] + 1;
			while(*o) switch(*o++) {
				case 'm':
					if(set_mode("-m") < 0) return -2;
					goto first_loop;
				case 'h':
					print_usage(argv[0]);
					return 0;
				case '-':
					if(*o) {
						if(strcmp(o, "mode") == 0) {
							if(set_mode("--mode") < 0) return -2;
							goto first_loop;
						} else if(strcmp(o, "help") == 0) {
							print_usage(argv[0]);
							return 0;
						}
					} else goto options_parse_end;
			}
		}
		i++;
	}

options_parse_end:
	if(argc < 3) {
		print_usage(argv[0]);
		return -1;
	}
	if(argv[2][1]) {
		fprintf(stderr, "%s: The type can only take one character\n", argv[0]);
		return 1;
	}
	switch(*(argv[2])) {
		case 'b':
			mode |= S_IFBLK;
			break;
		case 'c':
			mode |= S_IFCHR;
			break;
		case 'p':
			mode |= S_IFIFO;
			break;
		default:
			fprintf(stderr, "%s: Invalid type '%c'\n", argv[0], *(argv[2]));
			return 1;
	}
	dev_t dev = 0;
	if((mode & S_IFBLK) || (mode & S_IFCHR)) {
		if(argc < 5) {
			fprintf(stderr, "%s: Missing major and minor for type '%c'\n", argv[0], *(argv[2]));
			return 2;
		} else if(argc > 5) {
			fprintf(stderr, "%s: Too many operand for type '%c'\n", argv[0], *(argv[2]));
			return 2;
		}
		dev = makedev(atoi(argv[3]), atoi(argv[4]));
	} else if(argc > 3) {
		fprintf(stderr, "%s: Too many operand for type 'p'\n", argv[0]);
		return 2;
	}
	if(mknod(argv[1], mode, dev) < 0) {
		perror(argv[1]);
		return 3;
	}
	return 0;
}

