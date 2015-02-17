/*	dmesg - toolbox
	Copyright 2007-2015 PC GO Ld.
	Copyright 2015 libdll.so

	This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#if defined __GNU__ && defined __MACH__
#include <stdio.h>
extern int cat_main(int, char **);
#else
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
#endif

int dmesg_main(int argc, char **argv) {
#if defined __GNU__ && defined __MACH__
	char *cat_argv[] = { "cat", "/var/log/dmesg", NULL };
	int r = cat_main(2, cat_argv);
	if(r == 0) putchar('\n');
	return r;
#else
	char buffer[KLOG_BUF_LEN + 1];
	char *p = buffer;
	ssize_t ret;
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
