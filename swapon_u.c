/*	swapon - toolbox
	Copyright 2015-2016 Rivoreo

	This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#if defined __linux__ || defined __gnu_hurd__ || defined __SVR4
#ifdef __SVR4
#include <sys/stat.h>
#endif
#include <sys/swap.h>
#endif
#include <string.h>
#include <errno.h>

#if defined __linux__ || defined __gnu_hurd__
/* XXX These need to be obtained from kernel headers. See b/9336527 */
#define SWAP_FLAG_PREFER        0x8000
#define SWAP_FLAG_PRIO_MASK     0x7fff
#define SWAP_FLAG_PRIO_SHIFT    0
#define SWAP_FLAG_DISCARD       0x10000
#else
#ifdef __SVR4
static int swapon(char *path) {
	struct swapres res = { .sr_name = path };
	return swapctl(SC_ADD, &res);
}
#endif
#define swapon(P,F) swapon(P)
#endif

static void print_usage(const char *name) {
	fprintf(stderr, "Usage: %s [-v]"
#if defined __linux__ || defined __gnu_hurd__
		"[-p <prio>] "
#endif
		"<path>\n", name);
#if defined __linux__ || defined __gnu_hurd__
	fprintf(stderr, "	prio must be between 0 and %d\n", SWAP_FLAG_PRIO_MASK);
#endif
}

#if defined __linux__ || defined __gnu_hurd__
static int parse_prio(const char *prio_str) {
	unsigned long int p = strtoul(prio_str, NULL, 10);
	return (p > SWAP_FLAG_PRIO_MASK)? -1 : (int)p;
}
#endif

int swapon_main(int argc, char **argv) {
#if defined __linux__ || defined __gnu_hurd__
	int flags = 0;
	int prio;
#endif
	int verbose = 0;

	opterr = 0;

	while(1) {
		int c = getopt(argc, argv, "h"
#if defined __linux__ || defined __gnu_hurd__
			"p:"
#endif
			"v");
		if(c == -1) break;
		switch(c) {
#if defined __linux__ || defined __gnu_hurd__
			case 'p':
				if(!optarg) {
					print_usage(argv[0]);
					return -EINVAL;
				}
				prio = parse_prio(optarg);
				flags |= SWAP_FLAG_PREFER;
				flags |= (prio << SWAP_FLAG_PRIO_SHIFT) & SWAP_FLAG_PRIO_MASK;
				break;
#endif
			case 'v':
				verbose = 1;
				break;
			case 'h':
				print_usage(argv[0]);
				return 0;
			case '?':
				fprintf(stderr, "Unknown option: '-%c'\n", optopt);
				return -EINVAL;
		}
	}

	if(optind != argc - 1) {
		print_usage(argv[0]);
		return -EINVAL;
	}

	if(swapon(argv[argc - 1], flags) < 0) {
		int e = errno;
		fprintf(stderr, "swapon failed for %s, %s\n", argv[argc - 1], strerror(e));
		return -e;
	}

	return 0;
}
