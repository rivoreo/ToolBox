/*	hostname - toolbox
	Copyright 2015 libdll.so

	This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <limits.h>
#include <string.h>
#include <errno.h>

#ifdef _WIN32
#include <windows.h>
#include <winsock2.h>
#define HOST_NAME_MAX 16
#ifndef _WIN32_WCE
static int _sethostname(const char *name, size_t len) {
	if(len > HOST_NAME_MAX) {
		//errno = EINVAL;
		errno = ENAMETOOLONG;
		return -1;
	}
#if 0
	char buffer[len + 1];
	memcpy(buffer, name, len)[len] = 0;
	return SetComputerNameA(buffer) ? 0 : -1;
#else
	wchar_t buffer[len + 1];
	int wlen = mbstowcs(buffer, name, len);
	if(wlen < 0) {
		errno = EINVAL;
		return -1;
	}
	buffer[wlen] = 0;
	return SetComputerNameW(buffer) ? 0 : -1;
#endif
}
#define sethostname _sethostname
#endif
#else
#ifndef HOST_NAME_MAX
#define HOST_NAME_MAX _POSIX_HOST_NAME_MAX
#endif
#endif

static void print_usage(const char *name) {
	fprintf(stderr, "Usage:\n"
		"	%s [-f|-s]\n"
		"	%s -F <file>\n"
		"	%s <hostname>\n\n",
		name, name, name);
}

static char *get_name_from_file(const char *filename) {
	static char buffer[HOST_NAME_MAX+1];
	int fd = open(filename, O_RDONLY);
	if(fd == -1) return NULL;
	while(1) {
		char *p = buffer;
		int s = read(fd, buffer, HOST_NAME_MAX);
		if(s < 0) {
			int e = errno;
			close(fd);
			errno = e;
			return NULL;
		}
		buffer[s] = 0;
		/* support note */
		if(*p == '#') {
			while(*++p != '\n') if(!*p) {
				close(fd);
				return p;
			}
			p++;
			if(s == HOST_NAME_MAX) {
				lseek(fd, p - buffer, SEEK_SET);
				continue;
			}
		}
		char *pp = p;
		while(*pp) {
			/* End of the hostname */
			if(*pp == '\n') {
				*pp = 0;
				if(pp != buffer && pp[-1] == '\r') pp[-1] = 0;
				break;
			}
			pp++;
		}
		close(fd);
		return p;
	}
}

int hostname_main(int argc, char **argv) {
	const char *filename = NULL;
	int short_name = 1;
	while(1) {
		int c = getopt(argc, argv, "F:fsh");
		if(c == -1) break;
		switch(c) {
			case 'F':
				filename = optarg;
				break;
			case 'f':
				short_name = 0;
				break;
			case 's':
				short_name = 1;
				break;
			case 'h':
				print_usage(argv[0]);
				return 0;
			case '?':
				return -1;
		}
	}
	/* If user input excess arguments */
	if(argc > optind + !filename) {
		print_usage(argv[0]);
		return -1;
	}

	/* Get user input hostname */
	const char *hostname = argv[optind];
	if(!hostname) {
		if(filename) {
			hostname = get_name_from_file(filename);
			if(!hostname) {
				perror(filename);
				return 1;
			}
		} else {
			char hostname[HOST_NAME_MAX+1];
			if(!gethostname(hostname, sizeof hostname) < 0) {
				perror("gethostname");
				return 1;
			}
			if(short_name) {
				char *p = hostname;
				while(*p) {
					if(*p == '.') {
						*p = 0;
						break;
					}
					p++;
				}
			}
			puts(hostname);
			return 0;
		}
	}
	if(sethostname(hostname, strlen(hostname)) < 0) {
		perror("sethostname");
		return 1;
	}
	return 0;
}
