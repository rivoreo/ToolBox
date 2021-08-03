/*	touch - toolbox
	Copyright 2007-2015 PC GO Ld.
	Copyright 2015-2021 Rivoreo

	This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#define _ATFILE_SOURCE
#include <sys/stat.h>
#include "timefunc.h"
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <stdlib.h>
#ifdef _NO_UTIMENSAT
#include <sys/time.h>
#ifdef _NO_UTIMES
#include <utime.h>

static int utimes(const char *filename, const struct timeval *times) {
	struct utimbuf buf = {
		.actime = times[0].tv_sec,
		.modtime = times[1].tv_sec
	};
	return utime(filename, &buf);
}
#endif
#endif

#ifndef _NO_UTIMENSAT
#ifndef AT_FDCWD
#define AT_FDCWD -100
#endif
#ifndef AT_SYMLINK_NOFOLLOW
#define AT_SYMLINK_NOFOLLOW 0x100
#endif
#endif

static void usage(void) {
	fprintf(stderr, "Usage: touch"
#if defined _WIN32 && !defined _WIN32_WNT_NATIVE
		".exe"
#endif
		" [-am"
#ifndef _NO_UTIMENSAT
		"hl"
#endif
		"] [-t <timestamp>] <file>\n");
}

int touch_main(int argc, char *argv[]) {
	int i, aflag = 0, mflag = 0, debug = 0, end_of_options = 0;
	char *path = NULL;
#ifdef _NO_UTIMENSAT
	int tflag = 0;
	struct timeval specified_time, times[2];
	specified_time.tv_sec = time(NULL);
	specified_time.tv_usec = 0;
#else
	int flags = 0;
	struct timespec specified_time, times[2];
	specified_time.tv_sec = 0;
	specified_time.tv_nsec = UTIME_NOW;
#endif
	for(i = 1; i < argc; i++) {
		if(!end_of_options && argv[i][0] == '-' && argv[i][1]) {
			/* an option */
			const char *arg = argv[i] + 1;
			while(arg[0]) {
				switch(arg[0]) {
					case 'a': aflag = 1; break;
					case 'm': mflag = 1; break;
					case 't':
						if(i + 1 >= argc) {
							usage();
							return 1;
						}
						char *a = argv[++i];
						long int nsec = 0;
						char *dot = strchr(a, '.');
						if(dot) {
							char nsec_buffer[10];
							size_t len = strlen(dot + 1);
							if(len > 9) len = 9;
							memcpy(nsec_buffer, dot + 1, len);
							memset(nsec_buffer + len, '0', 9 - len);
							nsec_buffer[9] = 0;
							nsec = atol(nsec_buffer);
							*dot = 0;
						}
						specified_time.tv_sec = atol(a);
						if(!dot && specified_time.tv_sec == 0) {
							fprintf(stderr, "touch: invalid time_t\n");
							return 1;
						}
#ifdef _NO_UTIMENSAT
						tflag = 1;
						specified_time.tv_usec = nsec / 1000;
#else
						specified_time.tv_nsec = nsec;
#endif
						break;
#ifndef _NO_UTIMENSAT
					case 'h':
					case 'l':
						flags |= AT_SYMLINK_NOFOLLOW;
						break;
#endif
					case 'd': debug = 1; break;
					case '-':
						if(arg[0]) {
							usage();
							return -1;
						} else {
							end_of_options = 1;
							break;
						}
					default:
						usage();
						return -1;
				}
				arg++;
			}
		} else {
			/* not an option, and only accept one filename */
			if(i + 1 != argc) {
				usage();
				return -1;
			}
			path = argv[i];
		}
	}

	if(!path) {
		fprintf(stderr, "touch: no file specified\n");
		return 1;
	}

	if(access(path, F_OK) < 0) {
		if(debug) fprintf(stderr, "File not exists\n");
		int fd = creat(path, 0666);
		if(fd != -1) close(fd);
	}

#ifdef _NO_UTIMENSAT
	if(!mflag && !aflag) {
		if(!tflag) {
			if(utimes(path, NULL) < 0) {
				perror(path);
				return 1;
			}
			return 0;
		}
		aflag = mflag = 1;
	}

	if(aflag) {
		times[0] = specified_time;
	} else {
		struct stat omit;
		if(stat(path, &omit) < 0) {
			perror(path);
			return 1;
		}
		times[0].tv_sec = omit.st_atime;
		times[0].tv_usec = 0;
	}

	if(mflag) {
		times[1] = specified_time;
	} else {
		struct stat omit;
		if(stat(path, &omit) < 0) {
			perror(path);
			return 1;
		}
		times[0].tv_sec = omit.st_mtime;
		times[0].tv_usec = 0;
	}

	int s = utimes(path, times);
#else
	if(!mflag && !aflag) aflag = mflag = 1;

	if(aflag) {
		times[0] = specified_time;
	} else {
		times[0].tv_nsec = UTIME_OMIT;
	}

	if(mflag) {
		times[1] = specified_time;
	} else {
		times[1].tv_nsec = UTIME_OMIT;
	}

	if(debug) {
		fprintf(stderr, "path = \"%s\"\n", path);
		fprintf(stderr, "times[0].tv_sec = %ld, times[0].tv_nsec = %ld\n", times[0].tv_sec, times[0].tv_nsec);
		fprintf(stderr, "times[1].tv_sec = %ld, times[1].tv_nsec = %ld\n", times[1].tv_sec, times[1].tv_nsec);
		fprintf(stderr, "flags = 0x%8.8x\n", flags);
	}

	int s = utimensat(AT_FDCWD, path, times, flags);
#endif
	if(s < 0) {
		perror(path);
		return 1;
	}
	return 0;
}
