/*	tee - toolbox
	Copyright 2007-2015 PC GO Ld.
	Copyright 2015 libdll.so

	This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#ifdef _WIN32
#include <windows.h>
#ifdef _WIN32_WCE
#define BUFFER_SIZE 1024
#else
#define BUFFER_SIZE 2048
#endif
#else
#define BUFFER_SIZE 4096
#endif

static struct fd_node {
	int fd;
	const char *name;
	int no_close;
	struct fd_node *next;
} *fd_list = NULL;

static int add_to_list(int fd, const char *name, int no_close) {
	struct fd_node *new_node = malloc(sizeof(struct fd_node));
	if(!new_node) return -1;
	new_node->fd = fd;
	new_node->name = name;
	new_node->no_close = no_close;
	new_node->next = fd_list;
	fd_list = new_node;
	return 0;
}

static void free_list() {
	while(fd_list) {
		if(!fd_list->no_close && close(fd_list->fd) < 0) {
			perror(fd_list->name);
		}
		fd_list = fd_list->next;
	}
}

int tee_main(int argc, char **argv) {
	char buffer[BUFFER_SIZE];
	int append = 0;
	int end_of_options = 0;
	int i = 1;
	while(argv[i]) {
		if(!end_of_options && argv[i][0] == '-' && argv[i][1]) {
			const char *o = argv[i] + 1;
			while(*o) switch(*o++) {
				case 'a':
					append = 1;
					if(i + 1 == argc) {
						fprintf(stderr, "%s: Warning: option '-a' is in the end of command line makes no sense\n", argv[0]);
					}
					break;
				case 'i':
					signal(SIGINT, SIG_IGN);
					break;
				case 'h':
					fprintf(stderr, "Usage: %s [<options>] [<file>] [...]\n\n"
						"Options:\n"
						"	-a, --append			Append the output to the files\n"
						"	-i, --ignore-interrupts		Ignore the SIGINT signal\n\n",
						argv[0]);
					return 0;
				case '-':
					if(*o) {
						if(strcmp(o, "append") == 0) append = 1;
						else if(strcmp(o, "ignore-interrupts") == 0) signal(SIGINT, SIG_IGN);
						else {
							fprintf(stderr, "%s: Invalid option '%s'\n", argv[0], argv[i]);
							return -1;
						}
					} else end_of_options = 1;
					break;
				default:
					fprintf(stderr, "%s: Invalid option '-%c'\n", argv[0], o[-1]);
					return -1;
			}
		} else {
			int fd, no_close;
			if(strcmp(argv[i], "-") == 0) {
				fd = STDOUT_FILENO;
				no_close = 1;
			} else {
				fd = open(argv[i], O_WRONLY | O_CREAT | (append ? O_APPEND : O_TRUNC), 0666);
				no_close = 0;
			}
			if(fd == -1) {
				fprintf(stderr, "%s: %s: %s\n", argv[0], argv[i], strerror(errno));
				continue;
			}
			if(add_to_list(fd, argv[i], no_close) < 0) {
				perror(argv[0]);
				free_list();
				return 2;
			}
		}
		i++;
	}

	if(add_to_list(STDOUT_FILENO, "stdout", 1) < 0) {
		perror(argv[0]);
		free_list();
		return 2;
	}

	struct fd_node *p;
	int rfd = STDIN_FILENO;
	//int s;
	while(1) {
		int s = read(rfd, buffer, BUFFER_SIZE);
		if(s < 0) {
			perror("read");
			free_list();
			return 1;
		}
		if(!s) break;
		for(p=fd_list; p; p=p->next) {
			if(write(p->fd, buffer, s) < 0) {
				fprintf(stderr, "%s: write: %s: %s\n", argv[0], p->name, strerror(errno));
			}
		}
	} //while(s == BUFFER_SIZE);

/*
	for(p=fd_list; p; p=p->next) {
		if(close(p->fd) < 0) {
			fprintf(stderr, "%s: close: %s: %s\n", argv[0], p->name, strerror(errno));
		}
	}
*/
	free_list();

	return 0;
}

