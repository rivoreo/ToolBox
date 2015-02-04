#ifndef _TIMEFUNC_H
#define _TIMEFUNC_H

#if defined _WIN32 && !defined _WIN32_WNT_NATIVE
#ifdef _WIN32_WCE
#include <sys/time.h>
#else
#include <sys/types.h>
struct timespec {
	time_t tv_sec;			// Seconds.
	long int tv_nsec;		// Nanoseconds.
};
#endif
#if defined _NO_UTIMENSAT && !defined _WIN32_WCE
extern int utimes(const char *, const struct timeval *);
#endif
#define UTIME_NOW ((1l << 30) - 1l)
#define UTIME_OMIT ((1l << 30) - 2l)
extern int utimensat(int, const char *, const struct timespec *, int flags);
#define CLOCK_MONOTONIC 1
extern int clock_gettime(clockid_t, struct timespec *);
#endif

#endif
