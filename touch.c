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
		"l"
#endif
		"] [-t <time_t>] <file>\n");
	exit(255);
}

int main(int argc, char *argv[]) {
	int i, fd, aflag = 0, mflag = 0, flags = 0, debug = 0;
	char *file = NULL;
#ifdef _NO_UTIMENSAT
	int tflag = 0;
	struct timeval specified_time, times[2];
	specified_time.tv_sec = time(NULL);
	specified_time.tv_usec = 0;
#else
	//int debug = 0;
	struct timespec specified_time, times[2];
//#ifdef _WIN32
//	specified_time.tv_sec = time(NULL);
//	specified_time.tv_nsec = 0;
//#else
	specified_time.tv_sec = 0;
	specified_time.tv_nsec = UTIME_NOW;
//#endif
#endif
	for(i = 1; i < argc; i++) {
		if(argv[i][0] == '-') {
			/* an option */
			const char *arg = argv[i]+1;
			while(arg[0]) {
				switch(arg[0]) {
					case 'a': aflag = 1; break;
					case 'm': mflag = 1; break;
					case 't':
						if(i + 1 >= argc) usage();
						specified_time.tv_sec = atol(argv[++i]);
						if(specified_time.tv_sec == 0) {
							fprintf(stderr, "touch: invalid time_t\n");
							exit(1);
						}
#ifdef _NO_UTIMENSAT
						tflag = 1;
						specified_time.tv_usec = 0;
#else
						specified_time.tv_nsec = 0;
#endif
						break;
#ifndef _NO_UTIMENSAT
					case 'l': flags |= AT_SYMLINK_NOFOLLOW; break;
#endif
					case 'd': debug = 1; break;
//#endif
					default:
						usage();
				}
				arg++;
			}
		} else {
			/* not an option, and only accept one filename */
			if(i + 1 != argc) usage();
			file = argv[i];
		}
	}

	if(!file) {
		fprintf(stderr, "touch: no file specified\n");
		exit(1);
	}

	if(access(file, F_OK) < 0) {
		if(debug) fprintf(stderr, "File not exists\n");
		if((fd = creat(file, 0666)) != -1) close(fd);
	}

#ifdef _NO_UTIMENSAT
	if(!mflag && !aflag) {
		if(!tflag) {
			int r = utimes(file, NULL);
			if(r < 0) perror(file);
			return r;
		}
		aflag = mflag = 1;
	}

	if(aflag) {
		times[0] = specified_time;
	} else {
		struct stat omit;
		stat(file, &omit);
		times[0].tv_sec = omit.st_atime;
		times[0].tv_usec = 0;
	}

	if(mflag) {
		times[1] = specified_time;
	} else {
		struct stat omit;
		stat(file, &omit);
		times[0].tv_sec = omit.st_mtime;
		times[0].tv_usec = 0;
	}

	int r = utimes(file, times);
#else
	if(!mflag && !aflag) aflag = mflag = 1;

	if(aflag) {
		times[0] = specified_time;
	} else {
//#ifdef _WIN32
//		struct stat omit;
//		stat(file, &omit);
//		times[0].tv_sec = omit.st_atime;
//		times[0].tv_nsec = 0;
//#else
		times[0].tv_nsec = UTIME_OMIT;
//#endif
	}

	if(mflag) {
		times[1] = specified_time;
	} else {
//#ifdef _WIN32
//		struct stat omit;
//		stat(file, &omit);
//		times[1].tv_sec = omit.st_mtime;
//		times[1].tv_nsec = 0;
//#else
		times[1].tv_nsec = UTIME_OMIT;
//#endif
	}

	if(debug) {
		fprintf(stderr, "file = %s\n", file);
		fprintf(stderr, "times[0].tv_sec = %ld, times[0].tv_nsec = %ld\n", times[0].tv_sec, times[0].tv_nsec);
		fprintf(stderr, "times[1].tv_sec = %ld, times[1].tv_nsec = %ld\n", times[1].tv_sec, times[1].tv_nsec);
		fprintf(stderr, "flags = 0x%8.8x\n", flags);
	}

	int r = utimensat(AT_FDCWD, file, times, flags);
#endif
	if(r < 0) perror(argv[0]);
	return r;
}
