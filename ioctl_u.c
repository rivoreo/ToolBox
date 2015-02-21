/*	ioctl - toolbox
	Copyright 2015 libdll.so

	This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <string.h>
//#ifdef __linux__
//#include <linux/kd.h>
//#include <linux/vt.h>
//#endif
//#include <pthread.h>
#ifndef _WIN32
#include <sys/ioctl.h>
#else
#include <windows.h>
#ifdef _WIN32_WNT_NATIVE
#include <nt.h>
#else
#include <winioctl.h>
#endif
#ifndef O_SYNC
#define O_SYNC 0x101000
#endif
#endif

int ioctl_main(int argc, char *argv[]) {
	int fd;
	int res;

	int read_only = 0;
	int length = -1;
	int arg_size = 4;
#ifndef _WIN32
	int direct_arg = 0;
#endif
	uint32_t ioctl_nr;
	void *ioctl_args;
	uint8_t *ioctl_argp;
	uint8_t *ioctl_argp_save;
	int rem;
	int no_close = 0;

	while(1) {
		int c = getopt(argc, argv, "rdl:a:h");
		if(c == EOF) break;
		switch(c) {
			case 'r':
				read_only = 1;
				break;
#ifndef _WIN32
			case 'd':
				direct_arg = 1;
				break;
#endif
			case 'l':
				length = strtol(optarg, NULL, 0);
				break;
			case 'a':
				arg_size = strtol(optarg, NULL, 0);
				break;
			case 'h':
				fprintf(stderr, "Usage: %s [-l <length>] [-a <argsize>] [-r"
#ifndef _WIN32
					"d"
#endif
					"h] <device> <ioctlnr> [<arg>] [...]\n"
					"	-l <length>   Length of io buffer\n"
					"	-a <argsize>  Size of each argument (1-8)\n"
					"	-r            Open device in read only mode\n"
#ifndef _WIN32
					"	-d            Direct argument (no iobuffer)\n"
#endif
					"	-h            Print help\n", argv[0]);
				return -1;
			case '?':
				//fprintf(stderr, "%s: invalid option -%c\n", argv[0], optopt);
				//exit(1);
				return 1;
		}
	}

	if(optind + 2 > argc) {
		fprintf(stderr, "%s: too few arguments\n", argv[0]);
		return 1;
	}

	if(strcmp(argv[optind], "-") == 0) {
		fd = STDIN_FILENO;
		no_close = 1;
	} else fd = open(argv[optind], (read_only ? O_RDWR : O_RDONLY) | O_SYNC);
	if(fd == -1) {
		fprintf(stderr, "%s: Cannot open %s, %s\n", argv[0], argv[optind], strerror(errno));
		return 1;
	}
	optind++;

	ioctl_nr = strtol(argv[optind], NULL, 0);
	optind++;

#ifndef _WIN32
	if(direct_arg) {
		arg_size = 4;
		length = 4;
	}
#endif
	if(length < 0) {
		length = (argc - optind) * arg_size;
	}
	if(length) {
		ioctl_args = calloc(1, length);

		ioctl_argp_save = ioctl_argp = ioctl_args;
		rem = length;
		while(optind < argc) {
			uint64_t tmp = strtoull(argv[optind], NULL, 0);
			if(rem < arg_size) {
				fprintf(stderr, "%s: too many arguments\n", argv[0]);
				//exit(1);
				//return 1;
				goto failed;
			}
			memcpy(ioctl_argp, &tmp, arg_size);
			ioctl_argp += arg_size;
			rem -= arg_size;
			optind++;
		}
	}
	printf("sending ioctl 0x%x", ioctl_nr);
	rem = length;
	while(rem--) {
		printf(" 0x%02x", *ioctl_argp_save++);
	}
	putchar('\n');

#ifdef _WIN32
	//int ioctl(int fd, int request, ...) {
	int ioctl(int fd, int request, void *buffer) {
#ifdef _WIN32_WNT_NATIVE
		IO_STATUS_BLOCK io_status;
		long int status = NtDeviceIoControlFile((void *)fd, NULL, NULL, NULL, &io_status, request, buffer, length, buffer, length);
		if(status < 0) {
			__set_errno_from_ntstatus(status);
			return -1;
		}
		length = io_status.Information;
#else
		unsigned long int rsize;
		if(!DeviceIoControl((void *)fd, request, buffer, length, buffer, length, &rsize, NULL)) return -1;
		length = rsize;
#endif
		return 0;
	}
#else
	if(direct_arg) res = ioctl(fd, ioctl_nr, *(uint32_t *)ioctl_args);
	else
#endif
	res = ioctl(fd, ioctl_nr, length ? ioctl_args : 0);
	if(res < 0) {
		fprintf(stderr, "ioctl 0x%x failed, %s\n", ioctl_nr, strerror(errno));
		//return 1;
		goto failed;
	}
	if(!no_close) close(fd);
	if(length) {
		printf("return buf:");
		ioctl_argp = ioctl_args;
		rem = length;
		while(rem--) {
			printf(" %02x", *ioctl_argp++);
		}
		putchar('\n');
	}
	if(length) free(ioctl_args);
	return 0;

failed:
	if(!no_close) close(fd);
	if(length) free(ioctl_args);
	return 1;
}
