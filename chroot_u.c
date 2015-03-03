/*	chroot - toolbox
	Copyright 2015 libdll.so

	This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <grp.h>
#include <pwd.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>

#define DEFAULT_SHELL "/bin/sh"

static void print_usage(const char *name) {
	fprintf(stderr, "Usage: %s [<options>] <newroot> [<command> [<arg> ...]]\n"
		"\nOptions:\n"
		"	-u, --user	Set user to use\n"
		"	-g, --group	Set group to use\n"
		"	-G, --groups	Set a group list as <g1>[,<g2>[,...]]\n\n", name);
}

/*
static int set_other_groups(const char *group_list) {
}
*/

int chroot_main(int argc, char **argv) {
	static struct option long_options[5] = {
		{ "user", 1, NULL, 'u' },
		{ "group", 1, NULL, 'g' },
		{ "groups", 1, NULL, 'G' },
		{ "help", 0, NULL, 'h' }
	};
	uid_t uid = 0;
	gid_t gid = 0;
	const char *group_list = NULL;
	while(1) {
		// Don't over scan
		int c = getopt_long(argc, argv, "+u:g:G:h", long_options, NULL);
		if(c == -1) break;
		switch(c) {
			case 'u':
				if(isdigit(*optarg)) uid = atoi(optarg);
				else {
					struct passwd *pw = getpwnam(optarg);
					if(!pw) {
						fprintf(stderr, "%s: %s: No such user\n", argv[0], optarg);
						return 1;
					}
					uid = pw->pw_uid;
				}
				break;
			case 'g':
				if(isdigit(*optarg)) gid = atoi(optarg);
				else {
					struct group *gr = getgrnam(optarg);
					if(!gr) {
						fprintf(stderr, "%s: %s: No such group\n", argv[0], optarg);
						return 1;
					}
					gid = gr->gr_gid;
				}
				break;
			case 'G':
				group_list = optarg;
				break;
			case 'h':
				print_usage(argv[0]);
				return 0;
			case '?':
				return -1;
		}
	}
	if(argc == optind) {
		print_usage(argv[0]);
		return -1;
	}

	if(chroot(argv[optind]) < 0) {
		perror("chroot");
		return 1;
	}
	if(chdir("/") < 0) {
		perror("chdir");
		return 1;
	}

	if(uid && setuid(uid) < 0) {
		perror("setuid");
		return 1;
	}
	if(gid && setgid(gid) < 0) {
		perror("setgid");
		return 1;
	}
#if 0
	if(group_list && set_other_groups(group_list) < 0) {
		// Error message already printed in that function
		return 1;
	}
#else
	if(group_list) {
		fprintf(stderr, "%s: set_other_groups: %s\n", argv[0], strerror(ENOSYS));
		return 1;
	}
#endif

	if(argc < optind) {
		char *shell = getenv("SHELL");
		argv[0] = shell ? : DEFAULT_SHELL;
		argv[1] = NULL;
	} else argv += optind + 1;
	execvp(argv[0], argv);
	perror("execvp");
	return 1;
}
