/*	dmesg - toolbox
	Copyright 2007-2015 PC GO Ld.
	Copyright 2015 libdll.so

	This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#include <string.h>
#if (defined __GNU__ && defined __MACH__) || (defined __sun && defined __SVR4)
#include <stdio.h>
#if defined __GNU__ && defined __MACH__
#include <unistd.h>
#include <fcntl.h>
#endif
extern int cat_main(int, char **);
#elif defined __linux__
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <sys/klog.h>

#define KLOG_BUF_SHIFT 17		/* CONFIG_LOG_BUF_SHIFT default */
#define KLOG_BUF_LEN (1 << KLOG_BUF_SHIFT)

#define KLOG_READ_ALL 3
#define KLOG_READ_CLEAR 4

#else

#ifdef __APPLE__
#include <AvailabilityMacros.h>
#if defined MAC_OS_X_VERSION_MIN_REQUIRED && MAC_OS_X_VERSION_MIN_REQUIRED >= 1050
#define _USE_APPLE_PROC_KMSGBUF
#endif
#endif

#ifdef _USE_APPLE_PROC_KMSGBUF
#include <libproc.h>
#include <limits.h>
#ifndef MSG_BSIZE
#define MSG_BSIZE (16*1024)
#endif
#else
#include <sys/types.h>
#include <sys/sysctl.h>
#include <sys/msgbuf.h>
#endif
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#endif

static void print_usage(const char *name) {
	fprintf(stderr, "Usage: %s"
#if !defined _USE_APPLE_PROC_KMSGBUF && !(defined __sun && defined __SVR4)
		" [--read-clear|-c]"
#endif
		"\n", name);
}

int dmesg_main(int argc, char **argv) {
	int clear = 0;
	if(argc == 2) {
		if(strcmp(argv[1], "-c") == 0 || strcmp(argv[1], "--read-clear") == 0) clear = 1;
		else if(strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
			print_usage(argv[0]);
			return 0;
		}
	}
#if (defined __GNU__ && defined __MACH__) || (defined __sun && defined __SVR4)
	char *cat_argv[] = { "cat",
#if defined __GNU__ && defined __MACH__
		"/var/log/dmesg",
#elif defined __sun && defined __SVR4
		"/var/adm/messages",
#else
#error "Preprocessor fatal"
#endif
		NULL };
	int r = cat_main(2, cat_argv);
	if(r == 0) {
		putchar('\n');
#if defined __GNU__ && defined __MACH__
		if(clear) {
			int fd = open("/var/log/dmesg", O_TRUNC);
			if(fd == -1 || close(fd) < 0) {
				perror("dmesg");
				return 1;
			}
		}
#endif
	}
	return r;
#else
	ssize_t ret;
#ifdef __linux__
	char buffer[KLOG_BUF_LEN + 1];
	char *p = buffer;
	int op = clear ? KLOG_READ_CLEAR : KLOG_READ_ALL;
	int n = klogctl(op, buffer, KLOG_BUF_LEN);
	if (n < 0) {
		perror("klogctl");
		return EXIT_FAILURE;
	}
#elif defined _USE_APPLE_PROC_KMSGBUF
	char buffer[MSG_BSIZE];
	char *p = buffer;
	int n = proc_kmsgbuf(buffer, sizeof buffer);
	if(!n) {
		perror("proc_kmsgbuf");
		return 1;
	}
#else
	size_t n;
	if(sysctlbyname("kern.msgbuf", NULL, &n, NULL, 0) < 0) {
		perror("sysctlbyname");
		return 1;
	}
	if(!n) return 0;		// ?
	char buffer[n + 2], *p = buffer;
	if(sysctlbyname("kern.msgbuf", buffer, &n, NULL, 0) < 0) {
		perror("sysctlbyname");
		return 1;
	}
	if(clear && sysctlbyname("kern.msgbuf_clear", NULL, NULL, &clear, sizeof clear) < 0) {
		perror("sysctlbyname");
		return 1;
	}
	//char *p = buffer;
	if(buffer[n - 1] != '\n') buffer[n++] = '\n';
	while(!*p && n) {
		p++;
		n--;
	}
#endif
	buffer[n] = 0;
	while((ret = write(STDOUT_FILENO, p, n))) {
		if(ret == -1) {
			if(errno == EINTR) continue;
			perror("write");
			return EXIT_FAILURE;
		}
		p += ret;
		n -= ret;
	}

	return 0;
#endif
}
