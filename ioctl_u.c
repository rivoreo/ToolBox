/*	ioctl - toolbox
	Copyright 2015-2021 Rivoreo

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
#if !defined _WIN32 && !defined _WINDOWSNT_NATIVE
#include <sys/ioctl.h>
#else
#include <windows.h>
#ifdef _WINDOWSNT_NATIVE
#include <nt.h>
#else
#include <winioctl.h>
#endif
#ifndef O_SYNC
#define O_SYNC 0x101000
#endif
#endif
#if defined __INTERIX && !defined strtoull
extern unsigned long long int strtouq(const char *, char **, int);
#define strtoull strtouq
#endif

int ioctl_main(int argc, char *argv[]) {
	int fd;
	int res;

	int read_only = 0;
	int verbose = 0;
	int length = -1;
	int arg_size = 4;
	int is_string = 0;
#if !defined _WIN32 && !defined _WINDOWSNT_NATIVE
	int direct_arg = 0;
#endif
	uint32_t ioctl_nr;
	void *ioctl_args;
	uint8_t *ioctl_argp;
	uint8_t *ioctl_argp_save;
	int rem;
	int no_close = 0;

	while(1) {
		int c = getopt(argc, argv, "rvdl:a:sh");
		if(c == -1) break;
		switch(c) {
			case 'r':
				read_only = 1;
				break;
			case 'v':
				verbose = 1;
				break;
#if !defined _WIN32 && !defined _WINDOWSNT_NATIVE
			case 'd':
				direct_arg = 1;
				break;
#endif
			case 'l':
				length = strtol(optarg, NULL, 0);
				break;
			case 'a':
				if(is_string) {
					fprintf(stderr, "%s: Option '-a' is conflict with '-s'\n", argv[0]);
					return -1;
				}
				arg_size = strtoul(optarg, NULL, 0);
				if(arg_size < 1 || arg_size > 8) {
					fprintf(stderr, "%s: Invalid argument to '-a', invalid range\n", argv[0]);
					return -1;
				}
				break;
			case 's':
				is_string = 1;
				arg_size = sizeof(char *);
				break;
			case 'h':
				fprintf(stderr, "Usage: %s [-l <length>] [-a <argsize> | -s] [-rv"
#if !defined _WIN32 && !defined _WINDOWSNT_NATIVE
					"d"
#endif
					"h] <device> <ioctlnr> [<arg>] [...]\n"
					"	-l <length>   Length of I/O buffer\n"
					"	-a <argsize>  Size of each argument (1-8)\n"
					"	-s            Argments are pointers to C string\n"
					"	-r            Open device in read only mode\n"
					"	-v            Be verbose\n"
#if !defined _WIN32 && !defined _WINDOWSNT_NATIVE
					"	-d            Direct argument (no iobuffer)\n"
#endif
					"	-h            Print help\n", argv[0]);
				return -1;
			case '?':
				//fprintf(stderr, "%s: invalid option -%c\n", argv[0], optopt);
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

	ioctl_nr = strtoul(argv[optind], NULL, 0);
	optind++;

#if !defined _WIN32 && !defined _WINDOWSNT_NATIVE
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
			union {
				uint8_t a1;
				uint16_t a2;
				uint32_t a4;
				uint64_t a8;
				char *ap;
			} tmp;
			if(rem < arg_size) {
				fprintf(stderr, "%s: too many arguments\n", argv[0]);
				//return 1;
				goto failed;
			}
			if(is_string) tmp.ap = argv[optind];
			else switch(arg_size) {
				case 1:
					tmp.a1 = strtoul(argv[optind], NULL, 0);
					break;
				case 2:
					tmp.a2 = strtoul(argv[optind], NULL, 0);
					break;
				case 3 ... 4:
					tmp.a4 = strtoul(argv[optind], NULL, 0);
					break;
				case 5 ... 8:
					tmp.a1 = strtoull(argv[optind], NULL, 0);
					break;
			}
			memcpy(ioctl_argp, &tmp, arg_size);
			ioctl_argp += arg_size;
			rem -= arg_size;
			optind++;
		}
	}
	if(verbose) {
		fprintf(stderr, "sending ioctl 0x%x", (unsigned int)ioctl_nr);
		rem = length;
		while(rem--) fprintf(stderr, " 0x%02x", *ioctl_argp_save++);
		fputc('\n', stderr);
	}

#if defined _WIN32 || defined _WINDOWSNT_NATIVE
	//int ioctl(int fd, int request, ...) {
	int ioctl(int fd, int request, void *buffer) {
#ifdef _WINDOWSNT_NATIVE
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
		fprintf(stderr, "ioctl 0x%x failed, %s\n", (unsigned int)ioctl_nr, strerror(errno));
		//return 1;
		goto failed;
	}
	if(!no_close) close(fd);
	printf("%d\n", res);
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
