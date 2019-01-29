/*	setconsole - toolbox
	Copyright 2015-2018 Rivoreo

	This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <linux/kd.h>
#include <linux/vt.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/ioctl.h>

static int activate_thread_switch_vc;

static void *activate_thread(void *arg) {
	int fd = (int)arg;
	while(activate_thread_switch_vc >= 0) {
		while(ioctl(fd, VT_ACTIVATE, activate_thread_switch_vc) < 0) {
			if(errno == EINTR) continue;
			fprintf(stderr, "ioctl(fd, VT_ACTIVATE, %d) failed, %s\n", activate_thread_switch_vc, strerror(errno));
			break;
		}
		if(activate_thread_switch_vc >= 0) sleep(1);
	}
	return NULL;
}


int setconsole_main(int argc, char *argv[]) {
	int fd;

	int mode = -1;
	int new_vc = 0;
	int close_vc = 0;
	int switch_vc = -1;
	int printvc = 0;
	char *ttydev = "/dev/tty0";

	while(1) {
		int c = getopt(argc, argv, "d:gtncv:poh");
		if(c == -1) break;
		switch (c) {
			case 'd':
				ttydev = optarg;
				break;
			case 'g':
				if(mode == KD_TEXT) {
					fprintf(stderr, "%s: cannot specify both -g and -t\n", argv[0]);
					return -1;
				}
				mode = KD_GRAPHICS;
				break;
			case 't':
				if(mode == KD_GRAPHICS) {
					fprintf(stderr, "%s: cannot specify both -g and -t\n", argv[0]);
					return -1;
				}
				mode = KD_TEXT;
				break;
			case 'n':
				new_vc = 1;
				break;
			case 'c':
				close_vc = 1;
				break;
			case 'v':
				switch_vc = atoi(optarg);
				break;
			case 'p':
				printvc |= 1;
				break;
			case 'o':
				printvc |= 2;
				break;
			case 'h':
				fprintf(stderr, "%s [-d <dev>] [-v <vc>] [-gtncpoh]\n"
						"  -d <dev>   Use <dev> instead of /dev/tty0\n"
						"  -v <vc>    Switch to virtual console <vc>\n"
						"  -g         Switch to graphics mode\n"
						"  -t         Switch to text mode\n"
						"  -n         Create and switch to new virtual console\n"
						"  -c         Close unused virtual consoles\n"
						"  -p         Print new virtual console\n"
						"  -o         Print old virtual console\n"
						"  -h         Print help\n", argv[0]);
				return 0;
			case '?':
				//fprintf(stderr, "%s: invalid option -%c\n", argv[0], optopt);
				return -1;
		}
	}
	if(mode == -1 && new_vc == 0 && close_vc == 0 && switch_vc == -1 && printvc == 0) {
		fprintf(stderr,"%s [-d <dev>] [-v <vc>] [-gtncpoh]\n", argv[0]);
		return -1;
	}

	fd = open(ttydev, O_RDWR | O_SYNC);
	if(fd == -1) {
		fprintf(stderr, "cannot open %s, %s\n", ttydev, strerror(errno));
		return 1;
	}

	if((printvc && !new_vc) || (printvc & 2)) {
		struct vt_stat vs;

		if(ioctl(fd, VT_GETSTATE, &vs) < 0) {
			fprintf(stderr, "ioctl(fd, VT_GETSTATE, &vs) failed, %s\n", strerror(errno));
		}
		printf("%d\n", vs.v_active);
	}

	if(new_vc) {
		int vtnum;
		if(ioctl(fd, VT_OPENQRY, &vtnum) < 0) {
			fprintf(stderr, "ioctl(fd, VT_OPENQRY, &vtnum) failed, %s\n", strerror(errno));
			return 1;
		}
		if(vtnum == -1) {
			fputs("got vtnum = -1 from VT_OPENQRY\n", stderr);
		}
		switch_vc = vtnum;
	}
	if(switch_vc != -1) {
		pthread_t thread;
		pthread_attr_t attr;
		activate_thread_switch_vc = switch_vc;
		pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
		pthread_create(&thread, &attr, activate_thread, (void *)fd);

		while(ioctl(fd, VT_WAITACTIVE, switch_vc) > 0) {
			if(errno == EINTR) continue;
			fprintf(stderr, "ioctl(fd, VT_WAITACTIVE, %d) failed, %s\n", switch_vc, strerror(errno));
			break;
		}
		activate_thread_switch_vc = -1;
		if(printvc & 1) printf("%d\n", switch_vc);
		close(fd);
		fd = open(ttydev, O_RDWR | O_SYNC);
		if (fd < 0) {
			fprintf(stderr, "cannot open %s, %s\n", ttydev, strerror(errno));
			return 1;
		}
	}
	if(close_vc && ioctl(fd, VT_DISALLOCATE, 0) < 0) {
		fprintf(stderr, "ioctl(fd, VT_DISALLOCATE, 0) failed, %s\n", strerror(errno));
	}
	if(mode != -1) {
		if(ioctl(fd, KDSETMODE, mode) < 0) {
			fprintf(stderr, "KDSETMODE %d failed, %s\n", mode, strerror(errno));
			return 1;
		}
	}
	return 0;
}
