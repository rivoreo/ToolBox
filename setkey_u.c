/*	setkey - toolbox
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
#include <getopt.h>
#include <sys/ioctl.h>

static void setkey_usage(const char *name) {
	fprintf(stderr, "Usage: %s [-t <table>] [-k <index>] [-v <value>] [-r] [-h]\n\n"
			"	-t <table>  Select table\n"
			"	-k <index>  Select key\n"
			"	-v <value>  Set entry\n"
			"	-r          Read current entry\n"
			"	-h          Print help\n\n", name);
}

#define TTYDEV "/dev/tty0"

int setkey_main(int argc, char *argv[]) {
	int fd;
	struct kbentry kbe;
	int did_something = 0;

	kbe.kb_table = 0;
	kbe.kb_index = -1;
	kbe.kb_value = 0;

	fd = open(TTYDEV, O_RDWR | O_SYNC);
	if(fd == -1) {
		//fprintf(stderr, "open " TTYDEV ": %s\n", strerror(errno));
		perror(TTYDEV);
		return 1;
	}

	while(1) {
		int c, ret;

		c = getopt(argc, argv, "t:k:v:hr");
		if(c == -1) break;

		switch (c) {
			case 't':
				kbe.kb_table = strtol(optarg, NULL, 0);
				break;
			case 'k':
				kbe.kb_index = strtol(optarg, NULL, 0);
				break;
			case 'v':
				kbe.kb_value = strtol(optarg, NULL, 0);
				ret = ioctl(fd, KDSKBENT, &kbe);
				if (ret < 0) {
					fprintf(stderr, "KDSKBENT %d %d %d failed: %s\n",
						kbe.kb_table, kbe.kb_index, kbe.kb_value, strerror(errno));
					return 1;
				}
				did_something = 1;
				break;
			case 'r':
				ret = ioctl(fd, KDGKBENT, &kbe);
				if (ret < 0) {
					fprintf(stderr, "KDGKBENT %d %d  failed: %s\n",
						kbe.kb_table, kbe.kb_index, strerror(errno));
					return 1;
				}
				printf("0x%x 0x%x 0x%x\n", kbe.kb_table, kbe.kb_index, kbe.kb_value);
				did_something = 1;
				break;
			case 'h':
				setkey_usage(argv[0]);
				return 1;
			case '?':
				//fprintf(stderr, "%s: invalid option -%c\n", argv[0], optopt);
				return 1;
		}
	}

	if(optind != argc || !did_something) {
		setkey_usage(argv[0]);
		return -1;
	}

	return 0;
}
