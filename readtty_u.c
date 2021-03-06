/*	readtty - toolbox
	Copyright 2015 libdll.so

	This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

//#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>

#ifndef TCGETA
#define TCGETA TIOCGETA
#endif

#ifndef TCSETA
#define TCSETA TIOCSETA
#endif

struct {
	char key;
	const char *chars;
} map[] = {
	{ '1', "_ -1?!,.:;\"'<=>()_" },
	{ '2', "Cabc2ABC" },
	{ '3', "Fdef3DEF" },
	{ '4', "Ighi4GHI" },
	{ '5', "Ljkl5JKL" },
	{ '6', "Omno6MNO" },
	{ '7', "Spqrs7PQRS" },
	{ '8', "Vtuv8TUV" },
	{ '9', "Zwxyz9WXYZ" },
	{ '0', "*+&0@/#*" },
};

static char next_char(char key, char current) {
	int i;
	char *next;
	for(i = 0; i < sizeof(map) / sizeof(map[0]); i++) {
		if(key == map[i].key) {
			next = strchr(map[i].chars, current);
			if(next && next[1]) return next[1];
			return map[i].chars[1];
		}
	}
	return key;
}

static char prev_char(char key, char current) {
	int i;
	char *next;
	for(i = 0; i < sizeof(map) / sizeof(map[0]); i++) {
		if(key == map[i].key) {
			next = strchr(map[i].chars+1, current);
			if(next && next[-1]) return next[-1];
			return map[i].chars[1];
		}
	}
	return key;
}

int readtty_main(int argc, char *argv[]) {
	char buf[1];
	struct termios ttyarg;
	struct termios savedttyarg;
	int nonblock = 0;
	int timeout = 0;
	int flush = 0;
	int phone = 0;
	char *accept = NULL;
	char *rejectstring = NULL;
	char last_char_in = 0;
	char current_char = 0;
	char *exit_string = NULL;
	int exit_match = 0;

	while(1) {
		int c = getopt(argc, argv, "nt:fa:r:pe:");
		if(c == EOF) break;
		switch(c) {
			case 't':
				timeout = atoi(optarg);
				break;
			case 'n':
				nonblock = 1;
				break;
			case 'f':
				flush = 1;
				break;
			case 'a':
				accept = optarg;
				break;
			case 'r':
				rejectstring = optarg;
				break;
			case 'p':
				phone = 1;
				break;
			case 'e':
				exit_string = optarg;
				break;
			case '?':
				//fprintf(stderr, "%s: invalid option -%c\n", argv[0], optopt);
				return 1;
		}
	}

	if(flush) tcflush(STDIN_FILENO, TCIFLUSH);
	ioctl(STDIN_FILENO, TCGETA, &savedttyarg);
	ttyarg = savedttyarg;
	ttyarg.c_cc[VMIN] = (timeout > 0 || nonblock) ? 0 : 1;	/* minimum of 0 chars */
	ttyarg.c_cc[VTIME] = timeout;				/* wait max 15/10 sec */
	ttyarg.c_iflag = BRKINT | ICRNL; 
	ttyarg.c_lflag &= ~(ECHO | ICANON);
	ioctl(STDIN_FILENO, TCSETA, &ttyarg);

	while(1) {
		int s = read(STDIN_FILENO, buf, 1);
		if(s <= 0) {
			if(phone) {
				if(current_char) {
					write(STDERR_FILENO, &current_char, 1);
					write(STDOUT_FILENO, &current_char, 1);
					if(exit_string && current_char == exit_string[exit_match]) {
						exit_match++;
						if(exit_string[exit_match] == 0) break;
					} else exit_match = 0;
					current_char = 0;
				}
				continue;
			}
			break;
		}
		if(accept && strchr(accept, buf[0]) == NULL) {
			if(rejectstring) {
				write(STDOUT_FILENO, rejectstring, strlen(rejectstring));
				break;
			}
			if(flush) tcflush(STDIN_FILENO, TCIFLUSH);
			continue;
		}
		if(phone) {
			//if(!isprint(buf[0])) {
			//  fprintf(stderr, "got unprintable character 0x%x\n", buf[0]);
			//}
			if(!buf[0]) {
				if(current_char) {
					current_char = prev_char(last_char_in, current_char);
					write(STDERR_FILENO, &current_char, 1);
					write(STDERR_FILENO, "\b", 1);
				}
				continue;
			}
			if(current_char && buf[0] != last_char_in) {
				write(STDERR_FILENO, &current_char, 1);
				write(STDOUT_FILENO, &current_char, 1);
				if(exit_string && current_char == exit_string[exit_match]) {
					exit_match++;
					if(exit_string[exit_match] == 0) break;
				} else exit_match = 0;
				current_char = 0;
			}
			last_char_in = buf[0];
			current_char = next_char(last_char_in, current_char);
			write(STDERR_FILENO, &current_char, 1);
			write(STDERR_FILENO, "\b", 1);
			continue;
		}
		write(STDOUT_FILENO, buf, 1);
		break;
	}
	ioctl(STDIN_FILENO, TCSETA, &savedttyarg);		/* reset changed tty arguments */

	return 0;
}
