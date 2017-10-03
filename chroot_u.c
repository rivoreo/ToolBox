/*	chroot - toolbox
	Copyright 2015-2017 Rivoreo

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

#define DEFAULT_SHELL "/bin/sh"

static void print_usage(const char *name) {
	fprintf(stderr, "Usage: %s [<options>] <newroot> [<command> [<arg> ...]]\n"
		"\nOptions:\n"
		"	-u, --user	Set user to use\n"
		"	-g, --group	Set group to use\n"
		"	-G, --groups	Set a group list as <g1>[,<g2>[,...]]\n"
		"	--no-chdir	Don't do a chdir(2) after chroot(2)\n\n", name);
}

static uid_t strtouid(const char *user) {
	char *endptr;
	uid_t uid = strtoul(user, &endptr, 10);
	if(!*endptr) return uid;
	struct passwd *u = getpwnam(user);
	return u ? u->pw_uid : (uid_t)-1;
}

static gid_t strtogid(const char *group) {
	char *endptr;
	gid_t gid = strtoul(group, &endptr, 10);
	if(!*endptr) return gid;
	struct group *gr = getgrnam(group);
	return gr ? gr->gr_gid : (gid_t)-1;
}

static int set_other_groups(const char *group_list) {
#ifdef __INTERIX
	fprintf(stderr, "setgroups: %s\n", strerror(ENOSYS));
	return -1;
#else
	char buffer[32];
	const char *p = group_list;
	gid_t gid_list[32];
	int i = 0, j = 0;
	while(i < 32 && j < 32) {
		if(!p[i] || p[i] == ',') {
			buffer[i] = 0;
			gid_t gid = strtogid(buffer);
			if(gid == (gid_t)-1) {
				fprintf(stderr, "%s: No such group\n", buffer);
				return -1;
			}
			gid_list[j++] = gid;

			if(p[i]) {
				p += i + 1;
				i = 0;
				continue;
			}

			if(setgroups(j, gid_list) < 0) {
				perror("setgroups");
				return -1;
			}
			return 0;
		}
		buffer[i] = p[i];
		i++;
	}
	fprintf(stderr, "Too many groups or group name too long\n");
	return -1;
#endif
}

int chroot_main(int argc, char **argv) {
	static struct option long_options[] = {
		[0] = { "user", 1, NULL, 'u' },
		[1] = { "group", 1, NULL, 'g' },
		[2] = { "groups", 1, NULL, 'G' },
		[3] = { "no-chdir", 0, NULL, 0 },
		[4] = { "help", 0, NULL, 'h' },
		{ NULL, 0, NULL, 0 }
	};
	uid_t uid = 0;
	gid_t gid = 0;
	const char *group_list = NULL;
	int no_chdir = 0;
	while(1) {
		// Don't over scan
		int option_index = 0;
		int c = getopt_long(argc, argv, "+u:g:G:h", long_options, &option_index);
		if(c == -1) break;
		switch(c) {
			case 0:
				switch(option_index) {
					case 3:
						no_chdir = 1;
						break;
					default:
						fprintf(stderr, "%s: option_index %d shouldn't appear here!\n",
							argv[0], option_index);
						return -1;
				}
				break;
			case 'u':
				uid = strtouid(optarg);
				if(uid == (uid_t)-1) {
					fprintf(stderr, "%s: %s: No such user\n", argv[0], optarg);
					return 1;
				}
				break;
			case 'g':
				gid = strtogid(optarg);
				if(gid == (gid_t)-1) {
					fprintf(stderr, "%s: %s: No such group\n", argv[0], optarg);
					return 1;
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

	if(group_list && set_other_groups(group_list) < 0) {
		// Error message already printed in that function
		return 1;
	}

	if(chroot(argv[optind]) < 0) {
		perror("chroot");
		return 1;
	}
	if(!no_chdir && chdir("/") < 0) {
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

	if(argc <= optind + 1) {
		char *shell = getenv("SHELL");
		argv[0] = shell ? : DEFAULT_SHELL;
		argv[1] = NULL;
	} else argv += optind + 1;
	execvp(argv[0], argv);
	perror("execvp");
	return 1;
}
