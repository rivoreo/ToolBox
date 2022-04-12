/*	mkdir - toolbox
	Copyright 2015-2022 Rivoreo

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or (at
	your option) any later version.

	This program is distributed in the hope that it will be useful, but
	WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	General Public License for more details.
*/

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <sys/stat.h>
#include <getopt.h>
#include "str2mode.h"

static void print_usage() {
	fprintf(stderr,
		"Usage: mkdir"
#if defined _WIN32 && !defined _WINDOWSNT_NATIVE
		".exe"
#endif
		" [<option>] <target> [...]\n"
#if !defined _WIN32 || defined _WINDOWSNT_NATIVE
		"    -m, --mode <mode>	set mode for newly created directories\n"
#endif
		"    -p, --parents	create parent directories as needed\n"
		"    -v, --verbose	print name for each created directory\n"
		"    --help		display usage and exit\n");
}

int mkdir_main(int argc, char *argv[]) {
	static struct option long_options[] = {
		{ "help", 0, NULL, 'h' },
		{ "mode", 1, NULL, 'm' },
		{ "parents", 0, NULL, 'p' },
		{ "verbose", 0, NULL, 'v' },
		{ NULL, 0, NULL, 0 }
	};
	int ret = 0;
#if !defined _WIN32 || defined _WINDOWSNT_NATIVE
	mode_t mode = (mode_t)-1;
#endif
	int recursive = 0;
	int verbose = 0;
	while(1) {
		int c = getopt_long(argc, argv, "hm:pv", long_options, NULL);
		if(c == -1) break;
		switch(c) {
			case 'h':
				print_usage();
				return 0;
			case 'm':
#if !defined _WIN32 || defined _WINDOWSNT_NATIVE
				mode = STR2MODE(optarg);
				if(mode == (mode_t)-1) {
					fprintf(stderr, "%s: Bad mode '%s'\n", argv[0], optarg);
					return -1;
				}
#endif
				break;
			case 'p':
				recursive = 1;
				break;
			case 'v':
				verbose = 1;
				break;
			case '?':
				return -1;
		}
	}
	if(argc <= optind) {
		print_usage();
		return -1;
	}

#if !defined _WIN32 || defined _WINDOWSNT_NATIVE
	mode_t orig_umask = umask(0);
	if(mode == (mode_t)-1) {
		umask(orig_umask & ~(S_IWUSR | S_IXUSR));
		mode = 0777;
	}
#endif

	char currpath[PATH_MAX], *pathpiece;
	struct stat st;
	do {
		char *path = argv[optind];
		if(recursive) {
			// reset path
			//strcpy(currpath, "");
			*currpath = 0;
			// create the pieces of the path along the way
			pathpiece = strtok(path, "/");
			if(path[0] == '/') {
				// prepend / if needed
				strcat(currpath, "/");
			}
			while(pathpiece != NULL) {
				if(strlen(currpath) + strlen(pathpiece) + 2/*NUL and slash*/ > PATH_MAX) {
					fprintf(stderr, "Invalid path specified: too long\n");
					return 1;
				}
				strcat(currpath, pathpiece);
				strcat(currpath, "/");
				if(stat(currpath, &st) < 0) {
#if defined _WIN32 && !defined _WINDOWSNT_NATIVE
					int sr = mkdir(currpath);
#else
					int sr = mkdir(currpath, mode);
#endif
					if(sr < 0) {
						fprintf(stderr, "mkdir failed for '%s', %s\n", currpath, strerror(errno));
						ret = 1;
						break;
					} else if(verbose) {
						fprintf(stdout, "created directory '%s'\n", currpath);
					}
				}
				pathpiece = strtok(NULL, "/");
			}
		} else {
#if defined _WIN32 && !defined _WINDOWSNT_NATIVE
			int sr = mkdir(path);
#else
			int sr = mkdir(path, mode);
#endif
			if(sr < 0) {
				fprintf(stderr, "mkdir failed for %s, %s\n", path, strerror(errno));
				ret = 1;
			} else if(verbose) {
				fprintf(stdout, "created directory '%s'\n", path);
			}
		}
	} while(++optind < argc);

#if !defined _WIN32 || defined _WINDOWSNT_NATIVE
	umask(orig_umask);
#endif

	return ret;
}
