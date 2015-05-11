/*	more - toolbox
	Copyright 2015 libdll.so

	This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#ifdef _WIN32
#include <windows.h>
#include <winioctl.h>
#include <conio.h>
#ifndef IOCTL_CONSOLE_GETSCREENROWS
#define IOCTL_CONSOLE_GETSCREENROWS CTL_CODE(FILE_DEVICE_CONSOLE, 7, METHOD_BUFFERED, FILE_ANY_ACCESS)
#endif
#ifndef IOCTL_CONSOLE_GETSCREENCOLS
#define IOCTL_CONSOLE_GETSCREENCOLS CTL_CODE(FILE_DEVICE_CONSOLE, 9, METHOD_BUFFERED, FILE_ANY_ACCESS)
#endif
#ifndef FILE_DEVICE_CONSOLE
#define FILE_DEVICE_CONSOLE 0x00000102
#endif
#else
#include <sys/ioctl.h>
#include <termios.h>

static struct winsize winsz;
static struct termios old, new;

/* Initialize new terminal i/o settings */
static void set_terminal(int echo) {
	tcgetattr(0, &old); /* grab old terminal i/o settings */
	new = old; /* make new settings same as old settings */
	new.c_iflag &= ~ICRNL; /* Translate carriage return to newline on input (unless IGNCR is set) */
	new.c_lflag &= ~ICANON; /* disable buffered i/o */
	new.c_lflag &= echo ? ECHO : ~ECHO; /* set echo mode */
	tcsetattr(0, TCSANOW, &new); /* use these new terminal i/o settings now */
}

/* Restore old terminal i/o settings */
static void reset_terminal(void) {
	tcsetattr(0, TCSANOW, &old);
}

/* Read 1 character - echo defines echo mode */
static char getch_(int echo) {
	char ch;
	set_terminal(echo);
	ch = getchar();
	reset_terminal();
	return ch;
}

/* Read 1 character without echo */
static char getch(void) {
	return getch_(0);
}

#if 0		/* Not used */
/* Read 1 character with echo */
static char getche(void) {
	return getch_(1);
}
#endif
#endif

static int page_col, page_row;
static char filename[1024];

static int usage(char *name) {
	fprintf(stdout, "Usage:\n"
		"%s [<options>] [<file>] [...]\n\n"
		"Options:\n"
		"	-V	display version information and exit\n\n", name);
	return 1;
}

static int read_more() {
	switch(getch()) {
		case 13:
			page_row++;
			/* Clean the current line */
			printf("\x1b[1K");
			/* Set the cursor to the start of the line */
			printf("\x1b[%d;0H",winsz.ws_row);
			break;
		case 32:
			page_row = winsz.ws_row;
			/* Clean the current line */
			printf("\x1b[1K");
			/* Set the cursor to the start of the line */
			printf("\x1b[%d;0H",winsz.ws_row);
			break;
		case 'q':
			putchar('\n');
			exit(0);
		default:
			/* Clean the current line */
			printf("\x1b[1K");
			/* Set the cursor to the start of the line */
			printf("\x1b[%d;0H",winsz.ws_row);
			/* If none input, return 1 */
			return 1;
	}
	return 0;
}

static int read_file(FILE *fp, int use_pipe) {
	char line[page_col];
	int seek;
	if(!use_pipe) {
		struct stat filestat;
		lstat(filename, &filestat);
		while(fgets(line, page_col, fp) != NULL) {
			if(page_row != 1) {
				fprintf(stdout,"%s",line);
				//fflush(stdout);
				page_row--;
			} else {
				seek=ftell(fp);
				int percent = ((float)seek/(float)filestat.st_size) * 100;
				fprintf(stdout, "--More-- (%%%d)",percent);
				while(read_more() == 1) {
					fprintf(stdout, "--More-- (%%%d)",percent);
				}
			}

		}
	} else {
		char **buff;
		int i = 1;
		char line[page_col];
		/* First alloc */
		buff = (char **)malloc(sizeof(char *));
		while(fgets(line, page_col, fp) != NULL) {
			/* Alloc buff[i-1] */
			buff[i-1] = (char *)malloc(sizeof(char)*page_col+1);
			strcpy(buff[i-1], line);
			i++;
			/* realloc */
			buff = (char **)realloc(buff, sizeof(char *)*i);
			//printf("%s",buff[i-1]);
		}
		if(close(STDIN_FILENO)) {
			perror("STDIN_FILENO close faild");
			return errno;
		}
		if((fp = fopen("/dev/tty", "r")) == NULL) {
			perror("Error open TTY");
			return errno;
		}

		int line_num = i -1;
		int l = 0;
		for(i-- ; i > 0; i--) {
			if(page_row != 1) {
				fprintf(stdout,"%s",buff[l]);
				//fflush(stdout);
				page_row--;
			} else {
				int percent = ((float)l/(float)line_num) * 100;
				fprintf(stdout, "--More-- (%%%d)",percent);
				read_more();
			}
			l++;
		}
		/* Free memory */
		for(i=0; i<=line_num; i++) {
			free(buff[i]);
		}
		free(buff);
	}
	if(fclose(fp) != 0) {
		perror("Error to close");
		return errno;
	}
	return 0;
}


int more_main(int argc, char *argv[]) {
	int use_tty = isatty(STDOUT_FILENO);
	int use_pipe = !isatty(STDIN_FILENO);
	int opt;

	if(use_tty) {
#ifdef _WIN32
		unsigned long int row, col, rszie;
		if(!DeviceIoControl((void *)STDOUT_FILENO, IOCTL_CONSOLE_GETSCREENROWS, NULL, 0, &row, sizeof row, &rsize, NULL)) {
			return -errno;
		}
		if(!DeviceIoControl((void *)STDOUT_FILENO, IOCTL_CONSOLE_GETSCREENCOLS, NULL, 0, &col, sizeof col, &rszie, NULL)) {
			return -errno;
		}

		page_row = row;
		page_col = col;
#else
		/*
		 * Get terminal size 
		 *
		 * struct winsize {
		 *	unsigned short ws_row;
		 *	unsigned short ws_col;
		 *	unsigned short ws_xpixel;
		 *	unsigned short ws_ypixel;
		 *};
		 */
		if(ioctl(STDOUT_FILENO, TIOCGWINSZ, &winsz) == -1) {
			return -errno;
		}

		page_col = winsz.ws_col;
		page_row = winsz.ws_row;
#endif
	} else {
		//usage(argv[0]);
		fprintf(stderr, "%s: Not a terminal\n", argv[0]);
		return 1;
	}

	/* getopt */
	while((opt = getopt(argc, argv, "V")) != -1) {
		switch(opt) {
			case 'V':
				fprintf(stdout,"%s, From Toolbox by libdll.so\n", argv[0]);
				break;
			default:
				usage(argv[0]);
				return EXIT_FAILURE;
		}
	}
	if(optind == argc && !use_pipe) {
		usage(argv[0]);
		return 1;
	}

	/* 
	 * Now only support open a file or pipe
	 * So I dont use get opt now.
	 */
	FILE *fp;

	if(argc != 1) {
		strcpy(filename, argv[1]);
		if((fp = fopen(filename,"r")) == NULL) {
			perror("Error open file");
			return errno;
		}
	} else if(argc == 1 && !use_pipe) {
		usage(argv[0]);
		return 1;
	} else if(use_pipe) {
		if((fp = fdopen(STDIN_FILENO,"r")) == NULL) {
			perror("Error open STDIN_FILENO");
			return errno;
		}
	} else {
		fprintf(stderr, "%s: Cannot read from a terminal\n", argv[0]);
		return 1;
	}

	read_file(fp, use_pipe);

	return 0;
}
