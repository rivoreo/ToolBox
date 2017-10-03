/*	timetag - toolbox
	Copyright 2015-2017 Rivoreo

	This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#ifdef _WIN32_WCE
#define BUFFER_SIZE 1024
#else
#define BUFFER_SIZE 2048
#endif
#else
#define BUFFER_SIZE 4096
#endif

#define DEFAULT_FORMAT "%a %b %e %H:%M:%S %Z %Y "

static void print_usage(const char *name) {
	fprintf(stderr, "Usage: %s [-u] [<format>]\n", name);
}

static void *mem2chr(const void *s, int c1, int c2, size_t n) {
	char *p = (void *)s;
	unsigned int i = 0;
	while(i < n) {
		if(p[i] == c1 || p[i] == c2) return p + i;
		i++;
	}
	return NULL;
}

static void print_timetag(int utc, const char *format, const char *message, size_t msg_len) {
	char buffer[512];
	time_t t = time(NULL);
	struct tm tm;
	(utc ? gmtime_r : localtime_r)(&t, &tm);
	size_t tag_len = strftime(buffer, sizeof buffer, format, &tm);
	write(STDOUT_FILENO, buffer, tag_len);
	write(STDOUT_FILENO, message, msg_len);
	//write(STDOUT_FILENO, "\n", 1);
}

int timetag_main(int argc, char **argv) {
	int utc = 0;
	const char *format = NULL;
	int i = 1;
	int end_of_options = 0;
	while(argv[i]) {
		if(!end_of_options && argv[i][0] == '-') {
			const char *o = argv[i] + 1;
			while(*o) switch(*o++) {
				case 'u':
					utc = 1;
					putenv("TZ=UTC");
					tzset();
					break;
				case 'h':
					print_usage(argv[0]);
					return 0;
				case '-':
					if(o[1]) {
						fprintf(stderr, "%s: Invalid option '%s'\n", argv[0], argv[i]);
						return -1;
					} else end_of_options = 1;
					break;
				default:
					fprintf(stderr, "%s: Invalid option '-%c'\n", argv[0], o[-1]);
					return -1;
			}
		} else {
			if(format) {
				fprintf(stderr, "%s: Extra format '%s'\n", argv[0], argv[i]);
				return -1;
			}
			format = argv[i];
		}
		i++;
	}
	if(!format) format = DEFAULT_FORMAT;

	char buffer[BUFFER_SIZE];
	int s, ss = 0;
	int no_tag = 0;
	int r = 0;
	do {
		s = read(STDIN_FILENO, buffer + ss, sizeof buffer - ss);
		if(s < 0) {
			if(errno == EINTR) continue;
			perror("read");
			r = 1;
			break;
		}
		char *br = mem2chr(buffer + ss, 0, '\n', s);
		if(br) {
			int skip_len = 0;
			char *last_br;
			do {
				if(!*br) *br = '\n';
				br++;
				int msg_len = br - buffer - skip_len;
				if(no_tag) {
					write(STDOUT_FILENO, buffer + skip_len, msg_len);
					no_tag = 0;
				} else print_timetag(utc, format, buffer + skip_len, msg_len);
				last_br = br;
				br = mem2chr(br, 0, '\n', s - (br - (buffer + ss)));
				skip_len += msg_len;
				//fprintf(stderr, "br = %p, *br = '%c', skip_len = %d\n", br, br ? *br : 0, skip_len);
			} while(br);
			ss += s - skip_len;
			memmove(buffer, last_br, ss);
		} else {
			ss += s;
			if(ss == sizeof buffer) {
				print_timetag(utc, format, buffer, sizeof buffer);
				ss = 0;
				no_tag = 1;
			}
		}
	} while(s > 0);
	if(ss) {
		if(no_tag) write(STDOUT_FILENO, buffer, ss);
		else print_timetag(utc, format, buffer, ss);
		putchar('\n');
	}
	return r;
}
