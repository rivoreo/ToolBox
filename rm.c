/*	rm - toolbox
	Copyright 2007-2015 PC GO Ld.
	Copyright 2015 libdll.so

	This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/types.h>

#define OPT_RECURSIVE 1
#define OPT_FORCE 2

static void usage() {
	fprintf(stderr,"Usage: rm"
#if defined _WIN32 && !defined _WIN32_WNT_NATIVE
		".exe"
#endif
		" [-rR] [-f] <target> [...]\n");
	//return -1;
}

/* return -1 on failure, with errno set to the first error */
static int unlink_recursive(const char* name, int flags)
{
    struct stat st;
    DIR *dir;
    struct dirent *de;
    int fail = 0;

    /* is it a file or directory? */
    if (stat(name, &st) < 0)
        return ((flags & OPT_FORCE) && errno == ENOENT) ? 0 : -1;

    /* a file, so unlink it */
    if (!S_ISDIR(st.st_mode))
        return unlink(name);

    /* a directory, so open handle */
    dir = opendir(name);
    if (dir == NULL)
        return -1;

    /* recurse over components */
    errno = 0;
    while((de = readdir(dir)) != NULL) {
        char dn[PATH_MAX];
        if(strcmp(de->d_name, "..") == 0 || strcmp(de->d_name, ".") == 0) {
            continue;
	}
        sprintf(dn, "%s/%s", name, de->d_name);
        if(unlink_recursive(dn, flags) < 0) {
            fail = 1;
            break;
        }
        errno = 0;
    }
    /* in case readdir or unlink_recursive failed */
    if (fail || errno) {
        int save = errno;
        closedir(dir);
        errno = save;
        return -1;
    }

    /* close directory handle */
    if (closedir(dir) < 0)
        return -1;

    /* delete target directory */
    return rmdir(name);
}

int main(int argc, char *argv[])
{
	int ret;
	int i;
	int flags = 0;

/*
	if(argc < 2) {
		usage();
		return -1;
	}
*/
	/* check flags */
	while(1) {
		int c = getopt(argc, argv, "frR");
		if(c == -1) break;
		switch (c) {
			case 'f':
				flags |= OPT_FORCE;
				break;
			case 'r':
			case 'R':
				flags |= OPT_RECURSIVE;
				break;
		}
	}

	//printf("optind = %d\n", optind);
	if(optind < 1 || (optind >= argc && !(flags & OPT_FORCE))) {
		usage();
		return -1;
	}

	/* loop over the file/directory args */
	for (i = optind; i < argc; i++) {

		if (flags & OPT_RECURSIVE) {
			ret = unlink_recursive(argv[i], flags);
		} else {
//#ifdef _WIN32
			if(access(argv[i], F_OK) == 0 && access(argv[i], W_OK) != 0) {
				//if(flags & OPT_FORCE) chmod(argv[i], 666);
				if(!(flags & OPT_FORCE)) {
					//int a;
					printf("remove write-protected file \"%s\"? ", argv[i]);
/*
					a = getchar();
					if(a == 'Y' || a == 'y') chmod(argv[i], 666);
					else if(a == 'N' || a == 'n') return 1;
					else {
						fprintf(stderr, "Illegal input\n");
						return 1;
					}
*/
					switch(getchar()) {
						default:
							fprintf(stderr, "Illegal input\n");
						case 'N':
						case 'n':
							return 1;
						case 'Y':
						case 'y':
#if defined _WIN32 && !defined _WIN32_WNT_NATIVE
							chmod(argv[i], 666);
#endif
							break;
					}
				}
#if defined _WIN32 && !defined _WIN32_WNT_NATIVE
				else chmod(argv[i], 666);
#endif
			}
//#endif
			ret = unlink(argv[i]);
/*
			if (errno == ENOENT && !(flags & OPT_FORCE)) {
				return 0;
			}
*/
		}

		if(ret < 0 && (errno != ENOENT || !(flags & OPT_FORCE))) {
//#ifdef _WIN32_WCE
//			fprintf(stderr, "rm failed for %s\n", argv[i]);
//#else
			fprintf(stderr, "rm failed for %s, %s\n", argv[i], strerror(errno));
//#endif
			//if(errno != ENOENT || !(flags & OPT_FORCE)) return -1;
			return -1;
		}
	}

	return 0;
}

