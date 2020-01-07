/*	md5 - toolbox
	Copyright 2007-2015 PC GO Ld.
	Copyright 2015-2019 Rivoreo

	This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#ifdef _NO_OPENSSL
#include <md5.h>
#define MD5_Init MD5Init
#define MD5_Update MD5Update
#define MD5_Final MD5Final
#else
#include <openssl/md5.h>
#endif

#ifndef MD5_DIGEST_LENGTH
#define MD5_DIGEST_LENGTH 16
#endif

static int quiet = 0;

static void print_usage() {
	fprintf(stderr,"Usage: md5"
#if defined _WIN32 && !defined _WIN32_WNT_NATIVE
			".exe"
#endif
			" [-q] <file> [...]\n");
}

static int do_md5(const char *path) {
	unsigned int i;
	int fd;
	MD5_CTX md5_ctx;
	unsigned char md5[MD5_DIGEST_LENGTH];
	int no_close = 0;

	//fd = strcmp(path, "-") == 0 ? dup(STDIN_FILENO) : open(path, O_RDONLY);
	if(strcmp(path, "-") == 0) {
		fd = STDIN_FILENO;
		no_close = 1;
	} else fd = open(path, O_RDONLY);
	if(fd == -1) {
		fprintf(stderr,"Could not open %s, %s\n", path, strerror(errno));
		return -1;
	}

	MD5_Init(&md5_ctx);

	while(1) {
		char buf[4096];
		ssize_t rlen = read(fd, buf, sizeof buf);
		if(rlen == 0) break;
		if(rlen < 0) {
			int e = errno;
			if(!no_close) close(fd);
			fprintf(stderr,"Could not read %s, %s\n", path, strerror(e));
			return -1;
		}
		MD5_Update(&md5_ctx, buf, rlen);
	}
	if(!no_close && close(fd) < 0) {
		fprintf(stderr,"Could not close %s, %s\n", path, strerror(errno));
		return -1;
	}

	MD5_Final(md5, &md5_ctx);

	for(i = 0; i < (int)sizeof(md5); i++) printf("%02x", md5[i]);
	if(quiet) putchar('\n'); else printf("  %s\n", path);

	return 0;
}

int md5_main(int argc, char *argv[]) {
	int i = 1, end_of_options = 0, ret = 0;
	while(i < argc && !end_of_options) {
		if(argv[i][0] == '-') {
			int j;
			const char *o = argv[i] + 1;
			while(*o) switch(*o++) {
				case 'q':
					quiet = 1;
					break;
				case 'h':
					print_usage();
					return 0;
				case '-':
					if(*o) {
						if(strcmp(o, "quiet") == 0) {
							quiet = 1;
						} else {
							fprintf(stderr, "%s: Invalid option '%s'\n",
								argv[0], argv[i]);
							return -1;
						}
					} else end_of_options = 1;
					goto move_argv;
				default:
					fprintf(stderr, "%s: Invalid option '-%c'\n", argv[0], o[-1]);
					return -1;
			}
move_argv:
			for(j=i; j>0; j--) argv[j] = argv[j - 1];
			argc--;
			argv++;
		} else i++;
	}

	if(argc < 2) {
		print_usage();
		return -1;
	}

	/* loop over the file args */
	for (i = 1; i < argc; i++) {
		if(do_md5(argv[i]) < 0) ret = 1;
	}

	return ret;
}
