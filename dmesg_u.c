/*	dmesg - toolbox
	Copyright 2007-2015 PC GO Ld.
	Copyright 2015 libdll.so

	This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#if defined __GNU__ && defined __MACH__
#include <stdio.h>
extern int cat_main(int, char **);
#elif __linux__
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <sys/klog.h>
#include <string.h>

#define KLOG_BUF_SHIFT 17		/* CONFIG_LOG_BUF_SHIFT from our kernel */
#define KLOG_BUF_LEN (1 << KLOG_BUF_SHIFT)

#define KLOG_READ_ALL 3
#define KLOG_READ_CLEAR 4

#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#include <sys/msgbuf.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#endif

int dmesg_main(int argc, char **argv) {
#if defined __GNU__ && defined __MACH__
	char *cat_argv[] = { "cat", "/var/log/dmesg", NULL };
	int r = cat_main(2, cat_argv);
	if(r == 0) putchar('\n');
	return r;
#else
	ssize_t ret;
#ifdef __linux__
	char buffer[KLOG_BUF_LEN + 1];
	char *p = buffer;
	int n, op;

	if(argc == 2 && (strcmp(argv[1], "-c") == 0 || strcmp(argv[1], "--read-clear") == 0)) {
		op = KLOG_READ_CLEAR;
	} else {
		op = KLOG_READ_ALL;
	}

	n = klogctl(op, buffer, KLOG_BUF_LEN);
	if (n < 0) {
		perror("klogctl");
		return EXIT_FAILURE;
	}
#else
	int clear = 0;
	if(argc == 2 && (strcmp(argv[1], "-c") == 0 || strcmp(argv[1], "--read-clear") == 0)) {
		clear = 1;
	}

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
