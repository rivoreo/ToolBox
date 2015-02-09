/*	nohup - toolbox
	Copyright 2015 libdll.so

	This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#include <unistd.h>
#include <stdio.h>

int nohup_main(int argc, char *argv[]) {
	pid_t check;
	int c;

	if(argc == 1) {
		fprintf(stderr, "%s: missing operand\n"
				"Try 'nohup -h' for more information.\n", argv[0]);
		return -1;
	}

	while(1) {
		c = getopt(argc, argv, "h");
		if(c== EOF)
			break;
		switch(c) {
			case 'h':
				fprintf(stdout, "Usage: %s <command> [<arg>] [...]\n", argv[0]);
				return 0;
			case '?':
				fprintf(stdout, "Try 'nohup -h' for more information.\n");
				return 1;
		}
	}

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
}

