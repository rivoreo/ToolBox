/*	date - toolbox
	Copyright 2007-2015 PC GO Ld.
	Copyright 2015 libdll.so

	This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

//#include "timefunc.h"
#define _BSD_SOURCE		/* For settimeofday */
//#define _XOPEN_SOURCE		/* For strptime */
#define _POSIX_C_SOURCE 200112L	/* For gmtime_r and localtime_r */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
//#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
//#include <linux/android_alarm.h>
//#include <sys/ioctl.h>

#ifdef _WIN32
struct tm *gmtime_r(const time_t *, struct tm *);
struct tm *localtime_r(const time_t *, struct tm *);
#endif

static int settime(const char *s) {
	struct tm tm;
	int day = atoi(s);
	int hour;
	time_t t;
	//int fd;
	//struct timespec ts;
	struct timeval tv;

	while(*s && *s != '.') s++;

	if(*s) s++;

	hour = atoi(s);

	tm.tm_year = day / 10000 - 1900;
	tm.tm_mon = (day % 10000) / 100 - 1;
	tm.tm_mday = (day % 100);
	tm.tm_hour = hour / 10000;
	tm.tm_min = (hour % 10000) / 100;
	tm.tm_sec = (hour % 100);
	tm.tm_isdst = -1;

	t = mktime(&tm);

	//fd = open("/dev/alarm", O_RDWR);
	//ts.tv_sec = t;
	//ts.tv_nsec = 0;
	//ioctl(fd, ANDROID_ALARM_SET_RTC, &ts);

	tv.tv_sec = t;
	tv.tv_usec = 0;
	return settimeofday(&tv, NULL);
}

int main(int argc, char *argv[]) {
	int c;
	//int res;
	struct tm tm;
	time_t t;
	char strbuf[260];
	int useutc = 0;

	tzset();

	while(1) {
		c = getopt(argc, argv, "hus:");
		if(c == EOF)
			break;
		switch(c) {
			case 'h':
				goto usage;
			case 'u':
				useutc = 1;
				break;
			case 's':
				if(settime(optarg) < 0) {
					perror(argv[0]);
					return 3;
				}
				break;
			case '?':
				//fprintf(stderr, "%s: Invalid option -%c\n", argv[0], optopt);
				return 1;
		}
	}

	int hasfmt = argc > optind && argv[optind][0] == '+';
	int lindex = optind + (hasfmt ? 2 : 1);
	if(argc < lindex) {
		if(time(&t) < 0) {
			perror(argv[0]);
			return 2;
		}
/*
	} else if(argc == lindex) {
		struct timeval tv;
		if(!strptime(argv[optind], NULL, &tm)) {
			fprintf(stderr, "%s: Invaild date '%s'\n", argv[0], argv[optind]);
			return 2;
		}
		tv.tv_sec = mktime(&tm);
		tv.tv_usec = 0;
		printf("time %s -> %ld.%ld\n", argv[optind], (long int)tv.tv_sec, (long int)tv.tv_usec);
*/
	} else {
usage:
		fprintf(stderr,"Usage: %s [-s <YYYYmmdd.HHMMss>] [-u] [+<format>]\n", argv[0]);
		return -1;
	}

	if(useutc) {
		gmtime_r(&t, &tm);
		strftime(strbuf, sizeof strbuf, hasfmt ? argv[optind] + 1 : "%a %b %e %H:%M:%S GMT %Y", &tm);
	} else {
		localtime_r(&t, &tm);
		strftime(strbuf, sizeof strbuf, hasfmt ? argv[optind] + 1 : "%a %b %e %H:%M:%S %Z %Y", &tm);
	}
	puts(strbuf);

	return 0;
}
