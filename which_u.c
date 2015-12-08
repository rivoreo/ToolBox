/*	which - toolbox
	Copyright 2015 libdll.so

	This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>

#if defined _WIN32 && !defined _WIN32_WCE && !defined _WINDOWSNT_NATIVE
#define PATHS_SEPARATOR ';'
#else
#define PATHS_SEPARATOR ':'
#endif

static void print_usage(const char *name) {
	fprintf(stderr, "Usage: %s [-a|-s] <name> [...]\n", name);
}

int which(const char *path, const char *filename, char *rpath) {
	//fprintf(stderr, "function: which(%p<%s>, %p<%s>, %p)\n", path, path, filename, filename, rpath);
	char current_path[PATH_MAX + 1];
	int i = 0;
	if(!path || !filename) return -1;
	do {
		//char *p = current_path;
		struct stat st;
		int j = 0;
		while(path[i] != PATHS_SEPARATOR) {
			if(!path[i]) {
				i = 0;
				break;
			}
			if(j > PATH_MAX) {
				j = 0;
				while(path[++i] != PATHS_SEPARATOR) if(!path[i]) return -1;
				i++;
			}
			//*p++ = path[i++];
			current_path[j++] = path[i++];
		}
		if(i) i++;
		//if(p == current_path) *p++ = '.';
		//if(p[-1] != '/') *p++ = '/';
		if(!j) current_path[j++] = '.';
		if(current_path[j - 1] != '/') current_path[j++] = '/';
		size_t filename_len = strlen(filename);
		//size_t len = p - current_path + filename_len;
		size_t len = j + filename_len;
		if(len > PATH_MAX) continue;
		//memcpy(p, filename, filename_len + 1);
		memcpy(current_path + j, filename, filename_len + 1);
		if(access(current_path, X_OK) < 0) {
#if defined _WIN32 && !defined _WINDOWSNT_NATIVE
			if(len + 4 > PATH_MAX) continue;
			strcpy(current_path + len, ".exe");
			len += 4;
			if(access(current_path, X_OK) < 0)
#endif
				continue;
		}
		if(stat(current_path, &st) == 0 && S_ISREG(st.st_mode)) {
			if(rpath) memcpy(rpath, current_path, len + 1);
			return i;
		}
	} while(i);
	return -1;
}

int which_main(int argc, char **argv) {
	char candidate[PATH_MAX];
	int print_all = 0;
	int silent = 0;
	int r = 0;
	while(1) {
		int c = getopt(argc, argv, "ash");
		if(c == -1) break;
		switch(c) {
			case 'a':
				print_all = 1;
				break;
			case 's':
				silent = 1;
				break;
			case 'h':
				print_usage(argv[0]);
				return 0;
			case '?':
				return -1;
		}
	}
	if(optind == argc) {
		print_usage(argv[0]);
		return -1;
	}
	argv += optind;
	const char *path = getenv("PATH");
	if(!path) return 1;
	while(*argv) {
		int i = 0;
		do {
			int j = which(path + i, *argv, silent ? NULL : candidate);
			if(j < 0) {
				r++;
				break;
			}
			if(silent) break;
			puts(candidate);
			if(!j) break;
			i += j;
		} while(print_all);
		argv++;
	}
	return r;
}
