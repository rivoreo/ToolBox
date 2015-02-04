#define _BSD_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>

static int settime(const char *s) {
	struct tm tm;
	int day = atoi(s);
	int hour;
	time_t t;
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
	tv.tv_sec = t;
	tv.tv_usec = 0;
	return settimeofday(&tv, NULL);
}

int date_main(int argc, char *argv[]) {
	int c;
	struct tm tm;
	time_t t;
	char strbuf[260];
	int useutc = 0;

	tzset();

	while(1) {
		c = getopt(argc, argv, "us:");
		if(c == EOF) break;
		switch(c) {
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
				fprintf(stderr, "%s: Invalid option -%c\n", argv[0], optopt);
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
	} else {
		fprintf(stderr,"Usage: %s [-s <utc-time>] [-u] [+<format>]\n", argv[0]);
		return -1;
	}

	if(useutc) {
		gmtime_r(&t, &tm);
		strftime(strbuf, sizeof(strbuf), (hasfmt ? argv[optind] + 1 : "%a %b %e %H:%M:%S GMT %Y"), &tm);
	} else {
		localtime_r(&t, &tm);
		strftime(strbuf, sizeof(strbuf), (hasfmt ? argv[optind] + 1 : "%a %b %e %H:%M:%S %Z %Y"), &tm);
	}
	puts(strbuf);

	return 0;
}
