/*	hostname - toolbox
	Copyright 2015-2022 Rivoreo

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or (at
	your option) any later version.

	This program is distributed in the hope that it will be useful, but
	WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	General Public License for more details.
*/

#include <unistd.h>
#include <getopt.h>
#include <stdio.h>
#include <fcntl.h>
#include <limits.h>
#include <string.h>
#include <errno.h>

#ifdef _WIN32
#include <windows.h>
#include <winsock2.h>
#define HOST_NAME_MAX 16
static int _sethostname(const char *name, size_t len) {
	if(len > HOST_NAME_MAX) {
		//errno = EINVAL;
		errno = ENAMETOOLONG;
		return -1;
	}
#ifdef _WIN32_WCE
	/* The Winsock2 for Windows CE have a sethostname implementation
	 * but quite incompatible from the BSD sethostname(3) */

	extern size_t strnlen(const char *, size_t);

	size_t name_len = strnlen(name, len);
	if(!name_len) {
		errno = EINVAL;
		return -1;
	}
	if(len > name_len) len = name_len;
	char buffer[len + 1];
	memcpy(buffer, name, len);
	buffer[len] = 0;
	return sethostname(buffer, len + 1);
#else
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
#endif
}
#define sethostname _sethostname
#else
#include <sys/param.h>
#ifdef __SVR4
#include <netdb.h>
#ifdef __sun
extern int sethostname(char *, int);
#endif
#endif		/* __SVR4 */
#ifndef HOST_NAME_MAX
//#if defined _SC_HOST_NAME_MAX && defined _POSIX_HOST_NAME_MAX && _POSIX_HOST_NAME_MAX != (-1)
#if 0
#define HOST_NAME_MAX (sysconf(_SC_HOST_NAME_MAX) > 0 ? : _POSIX_HOST_NAME_MAX)
#elif defined MAXHOSTNAMELEN
#define HOST_NAME_MAX (MAXHOSTNAMELEN-1)
#elif defined _POSIX_HOST_NAME_MAX && _POSIX_HOST_NAME_MAX != (-1)
#define HOST_NAME_MAX _POSIX_HOST_NAME_MAX
#else
#define HOST_NAME_MAX 16
#endif
#endif	/* !HOST_NAME_MAX */
#endif

static void print_usage(const char *name) {
	fprintf(stderr, "Usage:\n"
		"	%s [-f|-s]\n"
		"	%s {-F|--file} <file>\n"
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
		/* Support comment */
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
	static struct option long_options[3] = {
		{ "file", 1, NULL, 'F' },
		{ "help", 0, NULL, 'h' }
	};
	const char *filename = NULL;
	int short_name = 0;
	while(1) {
		int c = getopt_long(argc, argv, "F:fsh", long_options, NULL);
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
	char *hostname = argv[optind];
	if(!hostname) {
		if(filename) {
			hostname = get_name_from_file(filename);
			if(!hostname) {
				perror(filename);
				return 1;
			}
		} else {
			// Just print the current host name
			char hostname[HOST_NAME_MAX+1];
#if defined _WIN32 && !defined _WIN32_WCE
			WSADATA d;
			WSAStartup(MAKEWORD(2, 2), &d);
#endif
			if(gethostname(hostname, sizeof hostname) < 0) {
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
#ifdef __INTERIX
	fprintf(stderr, "%s: %s\n", argv[0], strerror(ENOSYS));
	return 1;
#else
	if(sethostname(hostname, strlen(hostname)) < 0) {
		perror("sethostname");
		return 1;
	}
	return 0;
#endif
}
