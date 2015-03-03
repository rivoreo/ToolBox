/*	service - toolbox
	Copyright 2015 libdll.so

	This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#define INIT_D_PATH "/etc/init.d/"

static char *env_names[] = {
	"LANG", "PATH", "TERM"
};

static void print_usage(const char *name) {
	fprintf(stderr, "Usage: %s <service> [<action>] [<options>]\n", name);
}

static char service_script[PATH_MAX+1];
static size_t name_len;
static char *env[sizeof env_names / sizeof(char *) + 1];

int service_main(int argc, char **argv) {
	if(argc < 2) {
		print_usage(argv[0]);
		return -1;
	}
	if(strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
		print_usage(argv[0]);
		return 0;
	}
	if(strcmp(argv[1], "--status-all") == 0) {
		// TODO
		return 1;
	}

	if(sizeof INIT_D_PATH + name_len > sizeof service_script) {
		fprintf(stderr, "%s: Service name too long\n", argv[0]);
		return 1;
	}
	memcpy(service_script + sizeof INIT_D_PATH - 1, argv[1], name_len + 1);
	if(access(service_script, X_OK) < 0) {
		fprintf(stderr, "%s: %s: Unrecognized service\n", argv[0], argv[1]);
		return 1;
	}
	int i;
	name_len = strlen(argv[1]);
	service_script[PATH_MAX+1] = INIT_D_PATH;
	char **p = env;
	for(i = 0; i < sizeof env_names / sizeof(char *); i++) {
		char *e = getenv(env_names[i]);
		if(e) *p++ = e;
	}
	*p = NULL;
	//execle(service_script, argv[0], argv[2], NULL, env);
	//perror(argv[1]);
	argv++;
	execve(service_script, argv, env);
	perror(argv[0]);
	return 1;
}
