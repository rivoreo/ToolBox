/*	nohup - toolbox
	Copyright 2015 libdll.so

	This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

int nohup_main(int argc, char *argv[]) {
	//pid_t check;
	int err_fd = -1;

	if(argc > 1 && argv[1][0] == '-') switch(argv[1][1]) {
		case 0:
			break;
		case '-':
			if(!argv[1][2]) {
				argv[1] = argv[0];
				argv++;
				argc--;
				break;
			}
			fprintf(stdout, "%s: invalid option '%s'\n"
				"Try '%s -h' for more information.\n",
				argv[0], argv[1], argv[0]);
			return 1;
		case 'h':
			fprintf(stdout, "Usage: %s <command> [<arg>] [...]\n", argv[0]);
			return 0;
		default:
			fprintf(stdout, "%s: invalid option '-%c'\n"
				"Try '%s -h' for more information.\n",
				argv[0], argv[1][1], argv[0]);
			return 1;
	}
	if(argc == 1) {
		fprintf(stderr, "%s: missing operand\n"
			"Try '%s -h' for more information.\n",
			argv[0], argv[0]);
		return -1;
	}

#if 0
	pid_t pid = fork();
	if(pid == -1) {
		perror("fork");
		return 126;
	}
	if(pid) {
		int r;
		if(waitpid(pid, &r, 0) < 0) {
			perror("waitpid");
			return 126;
		}
		if(WIFSIGNALED(r)) {
			fprintf(stderr, "%s: Child process was terminated by signal %d\n", WTERMSIG(r));
		}
		return r;
	}
#endif
	if(isatty(STDOUT_FILENO)) {
		int fd = open("nohup.out", O_RDWR | O_CREAT | O_APPEND, 0666);
		if(fd == -1) {
			const char *home = getenv("HOME");
			if(!home) {
				fprintf(stderr, "%s: HOME not defined\n", argv[0]);
				fprintf(stderr, "%s: Cannot open 'nohup.out'\n", argv[0]);
				return 126;
			}
			size_t home_len = strlen(home);
			char path[home_len + 1 + 9 + 1];
			memcpy(path, home, home_len);
			path[home_len] = '/';
			strcpy(path + home_len + 1, "nohup.out");
			fd = open(path, O_RDWR | O_CREAT | O_APPEND, 0666);
			if(fd == -1) {
				fprintf(stderr, "%s: Cannot open '%s', %s\n", argv[0], path, strerror(errno));
				return 126;
			} else fprintf(stderr, "%s: Appending output to '%s'\n", argv[0], path);
		} else fprintf(stderr, "%s: Appending output to 'nohup.out'\n", argv[0]);
		if(dup2(fd, STDOUT_FILENO) == -1) {
			perror("dup2");
			//perror(argv[0]);
			return 126;
		}
	}
	if(isatty(STDERR_FILENO)) {
		err_fd = dup(STDERR_FILENO);
		if(err_fd != -1) stderr = fdopen(err_fd, "w");
		if(dup2(STDOUT_FILENO, STDERR_FILENO) == -1) {
			perror("dup2");
			return 126;
		}
	}
	signal(SIGHUP, SIG_IGN);

#if 0
	check = fork();
	/* fprintf(stdout, "1pid is = %d\n", check); */
	if(check == -1) {
		perror("Fork error");
		return -1;
	}
	if(check == 0) {
		check = fork();
		/* fprintf(stdout, "2pid is = %d\n", check); */
		if(check == -1) {
			perror("Fork error");
			return 1;
		}
		if(check == 0) {
			argv++;
			/* fprintf(stdout, "execing\n"); */
			execvp(*argv, argv);
			perror("execerr");
			return 2;
		}
		return 0;
	}
	return 0;
#else
	argv++;
	execvp(*argv, argv);
	int e = errno;
	//perror("execvp");
	perror(argv[0]);
	if(err_fd != -1) close(err_fd);
	return e == ENOENT ? 127 : 126;
#endif
}
