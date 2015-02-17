/*	mkswap - toolbox
	Copyright 2015 libdll.so

	This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <ctype.h>

/* XXX This needs to be obtained from kernel headers. See b/9336527 */
struct linux_swap_header {
	char           bootbits[1024];		/* Space for disklabel etc. */
	uint32_t       version;
	uint32_t       last_page;
	uint32_t       nr_badpages;
	unsigned char  sws_uuid[16];
	unsigned char  sws_volume[16];
	uint32_t       padding[117];
	uint32_t       badpages[1];
};

#define MAGIC_SWAP_HEADER     "SWAPSPACE2"
#define MAGIC_SWAP_HEADER_LEN 10
#define MIN_PAGES             10

static unsigned char atoxb(const char *s) {
	unsigned char r = *s - (isalpha(*s) ? (islower(*s) ? 87 : 55) : '0');
	if(*++s) r = (*s - (isalpha(*s) ? (islower(*s) ? 87 : 55) : '0')) + r * 16;
	return r;
}

static int convert_uuid(unsigned char *uuid, const char *s) {
	int i;
	for(i=0; i<16; i++) {
		if(*s == '-') s++;
		if(!isxdigit(*s) || !isxdigit(s[1])) {
			//memset(uuid + i, 0, 16 - i);
			//return;
			return -1;
		}
		uuid[i] = atoxb(s);
		s += 2;
	}
	return 0;
}

int mkswap_main(int argc, char **argv) {
	int e = 0;
	int fd;
	ssize_t len;
	off_t swap_size;
	int pagesize = 0;
	struct linux_swap_header sw_hdr = { .version = 1 };

	while(1) {
		int c = getopt(argc, argv, "p:L:U:");
		if(c == EOF) break;
		switch(c) {
			case 'p':
				pagesize = atoi(optarg);
				if(!pagesize) {
					fprintf(stderr, "%s: Invalid page size\n", argv[0]);
					return -EINVAL;
				}
				break;
			case 'L':
				strncpy((char *)sw_hdr.sws_volume, optarg, sizeof sw_hdr.sws_volume);
				break;
			case 'U':
				if(convert_uuid(sw_hdr.sws_uuid, optarg) < 0) {
					fprintf(stderr, "%s: Invalid UUID\n", argv[0]);
					return -EINVAL;
				}
				break;
			case '?':
				return -EINVAL;
		}
	}

	if(argc != optind + 1) {
		fprintf(stderr, "Usage: %s "
#ifdef __linux__
			"[-p <page size>]"
#else
			"-p <page size>"
#endif
			" [-L <label>] [-U <UUID>] <filename>\n", argv[0]);
		return -EINVAL;
	}

	if(!pagesize) {
#ifdef __linux__
		pagesize = getpagesize();
#else
		fprintf(stderr, "%s: Page size not specified\n", argv[0]);
		return -EINVAL;
#endif
	}

	fd = open(argv[optind], O_WRONLY);
	if(fd == -1) {
		e = errno;
		fprintf(stderr, "Cannot open %s, %s\n", argv[optind], strerror(e));
		return -e;
	}

	/* Determine the length of the swap file */
	swap_size = lseek(fd, 0, SEEK_END);
	if(swap_size < MIN_PAGES * pagesize) {
		fprintf(stderr, "Swap file needs to be at least %dkB\n",
			(MIN_PAGES * pagesize) >> 10);
		e = -ENOSPC;
		goto err;
	}

	if(lseek(fd, 0, SEEK_SET)) {
		e = errno;
		fprintf(stderr, "Can't seek to the beginning of the file, %s\n", strerror(e));
		goto err;
	}

	sw_hdr.last_page = (swap_size / pagesize) - 1;
	len = write(fd, &sw_hdr, sizeof sw_hdr);
	if(len != sizeof sw_hdr) {
		e = errno;
		fprintf(stderr, "Failed to write swap header into %s, %s\n", argv[optind], strerror(e));
		goto err;
	}

	/* Write the magic header */
	if(lseek(fd, pagesize - MAGIC_SWAP_HEADER_LEN, SEEK_SET) < 0) {
		e = errno;
		fprintf(stderr, "Failed to seek into %s, %s\n", argv[optind], strerror(e));
		goto err;
	}
	len = write(fd, MAGIC_SWAP_HEADER, MAGIC_SWAP_HEADER_LEN);
	if(len != MAGIC_SWAP_HEADER_LEN) {
		e = errno;
		fprintf(stderr, "Failed to write magic swap header into %s, %s\n", argv[optind], strerror(e));
		goto err;
	}

	if(fsync(fd) < 0) {
		e = errno;
		fprintf(stderr, "Failed to sync %s, %s\n", argv[optind], strerror(e));
		goto err;
	}

	return 0;

err:
	close(fd);
	return -e;
}
