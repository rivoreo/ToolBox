/*	chmod - toolbox
	Copyright 2015 libdll.so

	This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include <sys/stat.h>

#include <unistd.h>
#include <time.h>

static void recurse_chmod(const char *path, int mode) {
    struct dirent *dp;
    DIR *dir = opendir(path);
    if(dir == NULL) {
        // not a directory, carry on
        return;
    }
    char *subpath = malloc(sizeof(char) * PATH_MAX);
    int pathlen = strlen(path);

    while((dp = readdir(dir)) != NULL) {
        if(strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0) continue;

        if(strlen(dp->d_name) + pathlen + 2/*NUL and slash*/ > PATH_MAX) {
            fprintf(stderr, "Invalid path specified: too long\n");
            exit(1);
        }

        strcpy(subpath, path);
        strcat(subpath, "/");
        strcat(subpath, dp->d_name);

        if(chmod(subpath, mode) < 0) {
            fprintf(stderr, "Unable to chmod %s: %s\n", subpath, strerror(errno));
            exit(1);
        }

        recurse_chmod(subpath, mode);
    }
    free(subpath);
    closedir(dir);
}

static void usage() {
	fprintf(stderr, "Usage: chmod"
#if defined _WIN32 && !defined _WIN32_WNT_NATIVE
		".exe"
#endif
		" [<option>] <mode> <file>\n");
	fprintf(stderr, "  -R, --recursive         change files and directories recursively\n");
	fprintf(stderr, "  --help                  display this help and exit\n");

	//return 10;
}

int main(int argc, char **argv) {
    int i;

	if(argc < 3 || strcmp(argv[1], "--help") == 0) {
		usage();
		return -1;
	}

    int recursive = (strcmp(argv[1], "-R") == 0 ||
                     strcmp(argv[1], "--recursive") == 0) ? 1 : 0;

	if(recursive && argc < 4) {
		usage();
		return 10;
	}

    if(recursive) {
        argc--;
        argv++;
    }

    int mode = 0;
    const char *s = argv[1];
    while(*s) {
        if(*s >= '0' && *s <= '7') {
            mode = (mode<<3) | (*s-'0');
        } else {
            fprintf(stderr, "Bad mode\n");
            return 10;
        }
        s++;
    }

    for(i = 2; i < argc; i++) {
        if(chmod(argv[i], mode) < 0) {
            fprintf(stderr, "Unable to chmod %s: %s\n", argv[i], strerror(errno));
            return 10;
        }
        if(recursive) {
            recurse_chmod(argv[i], mode);
        }
    }
    return 0;
}
