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
#include <getopt.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/wait.h>

#define INIT_D_PATH "/etc/init.d/"

static const char *env_names[] = {
	"LANG", "PATH", "TERM"
};

static const char *ignore_names[] = {
	"skeleton", "README", "rc", "rcS", "single", "reboot", "bootclean.sh", "halt", "killall", "single", "linuxconf", "kudzu"
};

static char service_script[PATH_MAX+1] = INIT_D_PATH;
static size_t name_len;
static char *env[sizeof env_names / sizeof(char *) + 1];

static void print_usage(const char *name) {
	fprintf(stderr, "Usage: %s <service> [<action>] [<options>]\n", name);
}

static int get_status(const char *pathname) {
	pid_t pid = fork();
	if(pid < 0) return '?';
	if(pid) {
		int status;
		if(waitpid(pid, &status, 0) < 0) return '?';
		if(WIFSIGNALED(status)) return '?';
		//fprintf(stderr, "%d\n", WEXITSTATUS(status));
		int e = WEXITSTATUS(status);
		if(e == 1) return '?';
		return e == 0 ? '+' : '-';
	} else {
		close(STDOUT_FILENO);
		close(STDERR_FILENO);
		execle(pathname, pathname, "status", NULL, env);
		exit(1);
	}
}

static int status_all() {
	struct dirent *de;
	struct stat s;
	DIR *d = opendir(INIT_D_PATH);
	int i;
first_loop:
	while((de = readdir(d))) {
		if((strcmp(de->d_name, ".") == 0) || (strcmp(de->d_name, "..") == 0)) continue;
		for(i = 0; i < sizeof ignore_names / sizeof(char *); i++) {
			if(strcmp(de->d_name, ignore_names[i]) == 0) goto first_loop;
		}
		//snprintf(tmp, sizeof(tmp), INIT_D_PATH "%s", de->d_name);
		//strcpy(service_script, INIT_D_PATH);
		//strcat(service_script, de->d_name);

		name_len = strlen(de->d_name);
		if(sizeof INIT_D_PATH + name_len > sizeof service_script) {
			continue;
		}
		memcpy(service_script + sizeof INIT_D_PATH - 1, de->d_name, name_len + 1);

		if(stat(service_script, &s) < 0) {
			fprintf(stderr, "stat %s failed: %s", service_script, strerror(errno));
			return -1;
		}
		if(!S_ISDIR(s.st_mode) && (s.st_mode & 0111)) {
			//if(access(tmp, X_OK) == 0)
			//	fprintf(stdout, "%s\n", de->d_name);
			printf(" [ %c ] %s\n", get_status(service_script), de->d_name);
		}
	}
	return 0;
}

static void init_env() {
	int i;
	char **p = env;
	/* How many options the env_names have. */
	for(i = 0; i < sizeof env_names / sizeof(char *); i++) {
		char *e = getenv(env_names[i]);
		/* Copy HEAD to env[i] */
		if(e) *p++ = e;
	}
	*p = NULL;
}

int service_main(int argc, char **argv) {
	static struct option long_options[] = {
		{ "help",	no_argument, 0, 'h' },
		{ "status-all",	no_argument, 0, 0 },
		{ 0,		0,	0,	0 }
	};
	while(1) {
		int option_index = 0;
		int c = getopt_long(argc, argv, "h", long_options, &option_index);
		if(c == -1) break;
		switch(c) {
			case 'h':
				print_usage(argv[0]);
				return -1;
			case 0:
				//printf("%s\n", long_options[option_index].name);
				if(strcmp(long_options[option_index].name, "status-all")== 0) {
					init_env();
					status_all();
					return 1;
				}
				break;
			case '?':
				return -1;
			default:
				print_usage(argv[0]);
				return 0;
		}
	}
	/* Check if no option, Avoid Segmentation fault */
	if(optind == argc) {
		print_usage(argv[0]);
		return 0;
	}
	name_len = strlen(argv[1]);
	if(sizeof INIT_D_PATH + name_len > sizeof service_script) {
		fprintf(stderr, "%s: Service name too long\n", argv[0]);
		return 1;
	}
	memcpy(service_script + sizeof INIT_D_PATH - 1, argv[1], name_len + 1);
	if(access(service_script, X_OK) < 0) {
		fprintf(stderr, "%s: %s: Unrecognized service\n", argv[0], argv[1]);
		return 1;
	}
	init_env();
	//execle(service_script, argv[0], argv[2], NULL, env);
	//perror(argv[1]);
	argv++;
	execve(service_script, argv, env);
	perror(argv[0]);
	return 1;
}
