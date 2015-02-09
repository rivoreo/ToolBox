#ifndef _TIMEFUNC_H
#define _TIMEFUNC_H

#define AT_FDCWD		-100
#define AT_SYMLINK_NOFOLLOW	0x100
#define UTIME_NOW	((1l << 30) - 1l)
#define UTIME_OMIT	((1l << 30) - 2l)

#if defined _WIN32 && !defined _WIN32_WNT_NATIVE
struct timespec{
	long int tv_sec;	// Seconds.
	long int tv_nsec;	// Nanoseconds.
};

#if defined _NO_UTIMENSAT && !defined _WIN32_WCE
extern int utimes(const char *, const struct timeval *);
#endif
extern int utimensat(int, const char *, const struct timespec *, int flags);
extern int clock_gettime(unsigned long int, struct timespec *);
#endif

#endif
