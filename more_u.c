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
#include <fcntl.h>
#include <errno.h>

#ifdef _WIN32
#include <windows.h>
#include <winioctl.h>
#ifdef _WIN32_WCE
#ifndef IOCTL_CONSOLE_SETMODE
#define IOCTL_CONSOLE_SETMODE CTL_CODE(FILE_DEVICE_CONSOLE, 1, METHOD_BUFFERED, FILE_ANY_ACCESS)
#endif
#ifndef IOCTL_CONSOLE_GETMODE
#define IOCTL_CONSOLE_GETMODE CTL_CODE(FILE_DEVICE_CONSOLE, 2, METHOD_BUFFERED, FILE_ANY_ACCESS)
#endif
#ifndef IOCTL_CONSOLE_GETSCREENROWS
#define IOCTL_CONSOLE_GETSCREENROWS CTL_CODE(FILE_DEVICE_CONSOLE, 7, METHOD_BUFFERED, FILE_ANY_ACCESS)
#endif
#ifndef IOCTL_CONSOLE_GETSCREENCOLS
#define IOCTL_CONSOLE_GETSCREENCOLS CTL_CODE(FILE_DEVICE_CONSOLE, 9, METHOD_BUFFERED, FILE_ANY_ACCESS)
#endif
#define CECONSOLE_MODE_ECHO_INPUT 0x0001
#define CECONSOLE_MODE_LINE_INPUT 0x0002
#define CECONSOLE_MODE_PROCESSED_OUTPUT 0x0004
#ifndef FILE_DEVICE_CONSOLE
#define FILE_DEVICE_CONSOLE 0x00000102
#endif
#else
#include <conio.h>
#endif		/* _WIN32_WCE */
#else
#include <sys/ioctl.h>
#include <termios.h>
#endif

#if !defined _WIN32 || defined _WIN32_WCE || defined _WINDOWSNT_NATIVE
/* Initialize new terminal i/o settings */
#ifdef _WIN32_WCE
unsigned long int old, new;

static void set_terminal(int echo) {
	unsigned long int rsize;
	DeviceIoControl((void *)STDIN_FILENO, IOCTL_CONSOLE_GETMODE, NULL, 0, &old, sizeof old, &rsize, NULL);
	new = old;
	new &= echo ? CECONSOLE_MODE_ECHO_INPUT : ~CECONSOLE_MODE_ECHO_INPUT;
	new &= ~CECONSOLE_MODE_LINE_INPUT;
	DeviceIoControl((void *)STDIN_FILENO, IOCTL_CONSOLE_SETMODE, &new, sizeof new, NULL, 0, &rsize, NULL);
}

static void reset_terminal() {
	unsigned long int rsize;
	DeviceIoControl((void *)STDIN_FILENO, IOCTL_CONSOLE_SETMODE, &old, sizeof old, NULL, 0, &rsize, NULL);
}
#else
static struct winsize winsz;
static struct termios old, new;

static void set_terminal(int echo) {
	tcgetattr(STDIN_FILENO, &old);			/* grab old terminal i/o settings */
	new = old;					/* make new settings same as old settings */
	new.c_iflag &= ~ICRNL;				/* Translate carriage return to newline on input (unless IGNCR is set) */
	new.c_lflag &= ~ICANON;				/* disable buffered i/o */
	new.c_lflag &= echo ? ECHO : ~ECHO;		/* set echo mode */
	tcsetattr(STDIN_FILENO, TCSANOW, &new);	/* use these new terminal i/o settings now */
}

/* Restore old terminal i/o settings */
static void reset_terminal(void) {
	tcsetattr(STDIN_FILENO, TCSANOW, &old);
}
#endif

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

#define SKIP_MULTI_BLACK_LINES (1 << 0)

unsigned int term_row;

static int page_col, page_row;
static char filename[1024];

static int usage(char *name) {
	fprintf(stdout, "Usage: %s [<options>] [<file>]\n\n"
		"Options:\n"
		"	-s	Squeeze multiple blank lines into a single line\n"
		"	-V	Display version information and exit\n\n", name);
	return 1;
}

static int read_more() {
	switch(getch()) {
		case 13:
			page_row++;
#ifdef _WIN32
			printf("\r		\r");
#else
			/* Clean the current line */
			printf("\x1b[1K");
			/* Set the cursor to the start of the line */
			printf("\x1b[%u;0H", term_row);
#endif
			break;
		case 32:
			page_row = term_row;
#ifdef _WIN32
			printf("\r		\r");
#else
			/* Clean the current line */
			printf("\x1b[1K");
			/* Set the cursor to the start of the line */
			printf("\x1b[%u;0H", term_row);
#endif
			break;
		case 'q':
			putchar('\n');
			exit(0);
		default:
#ifdef _WIN32
			printf("\r		\r");
#else
			/* Clean the current line */
			printf("\x1b[1K");
			/* Set the cursor to the start of the line */
			printf("\x1b[%u;0H", term_row);
#endif
			/* If none input, return 1 */
			return 1;
	}
	return 0;
}

static int read_file(FILE *fp, int use_pipe, int flags) {
	char line[page_col];
	int seek;
	off_t filesize = -1;
	int black_line = 0;
	if(!use_pipe) {
		struct stat filestat;
		stat(filename, &filestat);
		filesize = filestat.st_size;
	}
	while(fgets(line, page_col, fp) != NULL) {
		if((flags & SKIP_MULTI_BLACK_LINES)) {
			if(*line == '\n') {
				if(black_line) continue;
				else black_line = 1;
			} else black_line = 0;
		}
		if(page_row != 1) {
			fprintf(stdout,"%s",line);
			//fflush(stdout);
			page_row--;
		} else {
			if(use_pipe) {
				fprintf(stdout, "--More--");
			} else {
				seek = ftell(fp);
				int percent = ((float)seek / (float)filesize) * 100;
				fprintf(stdout, "--More-- (%%%d)", percent);
			}
			while(read_more() == 1) {
				fprintf(stdout, "--More--");
			}
		}

	}

#if 0
	// There is no need to read all data from a pipe that just for calculate the length
	} else {
		char **buff;
		int i = 1;
		char line[page_col];
		/* First alloc */
		buff = (char **)malloc(sizeof(char *));
		if(!buff) return -1;
		while(fgets(line, page_col, fp) != NULL) {
			/* Alloc buff[i-1] */
			buff[i-1] = (char *)malloc(sizeof(char)*page_col+1);
			strcpy(buff[i-1], line);
			i++;
			/* realloc */
			buff = (char **)realloc(buff, sizeof(char *)*i);
			if(!buff) return -1;
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
#endif
/*	Should close in main
	if(fclose(fp) != 0) {
		perror("Error to close");
		return errno;
	}
*/
	return 0;
}


int more_main(int argc, char *argv[]) {
	int use_tty = isatty(STDOUT_FILENO);
	int use_pipe = !isatty(STDIN_FILENO);
	int opt;

	if(use_tty) {
#ifdef _WINDOWSNT_NATIVE
		// From Windows 2000
		page_row = 31;
		page_col = 78;
#else
#ifdef _WIN32
#ifdef _WIN32_WCE
		unsigned long int row, col, rsize;
		if(!DeviceIoControl((void *)STDOUT_FILENO, IOCTL_CONSOLE_GETSCREENROWS, NULL, 0, &row, sizeof row, &rsize, NULL)) {
			return -errno;
		}
		if(!DeviceIoControl((void *)STDOUT_FILENO, IOCTL_CONSOLE_GETSCREENCOLS, NULL, 0, &col, sizeof col, &rsize, NULL)) {
			return -errno;
		}

		page_row = row;
		page_col = col;
#else
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		if(!GetConsoleScreenBufferInfo((void *)STDOUT_FILENO, &csbi)) {
			return 1;
		}
		page_row = csbi.srWindow.Right - csbi.srWindow.Left + 1;
		page_col = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
#endif
		fprintf(stderr, "%d, %d\n", page_col, page_row);
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
#endif
		term_row = page_row;
	} else {
		//usage(argv[0]);
		fprintf(stderr, "%s: Not a terminal\n", argv[0]);
		return 1;
	}

	int flags = 0;
	/* getopt */
	while((opt = getopt(argc, argv, "sV")) != -1) {
		switch(opt) {
			case 's':
				flags |= SKIP_MULTI_BLACK_LINES;
				break;
			case 'V':
				//fprintf(stdout,"%s, From ToolBox by libdll.so\nToolBox version: " VERSION "\n", argv[0]);
				fprintf(stdout,"%s, From ToolBox by libdll.so\n", argv[0]);
				return 0;
			default:
				usage(argv[0]);
				return EXIT_FAILURE;
		}
	}
	if(optind == argc && !use_pipe) {
		usage(argv[0]);
		return -1;
	}

	int r = 0;
	int fd = -1;
	/* 
	 * Now only support open a file or pipe
	 * So I dont use get opt now.
	 */
	FILE *fp;

	if(argc != 1) {
		strcpy(filename, argv[optind]);
		fp = fopen(filename, "r");
		if(!fp) {
			perror("Error open file");
			return 1;
		}
	} else if(argc == 1 && !use_pipe) {
		usage(argv[0]);
		return 1;
	} else if(use_pipe) {
#if defined _WIN32 && !defined _WINDOWSNT_NATIVE
		fprintf(stderr, "%s: Reading from stdin is not supported yet\n", argv[0]);
		return 1;
#else
		fd = dup(STDIN_FILENO);
		if(fd == -1) {
			perror("dup");
			return 1;
		}
//#if defined _WIN32 && !defined _WIN32_WCE
		//fp = stdin;
//#else
		fp = fdopen(fd, "r");
		if(!fp) {
			perror("Error open STDIN_FILENO");
			return 1;
		}
		if(close(STDIN_FILENO) < 0) {
			perror("close");
			return 1;
		}
//#endif
//#ifndef _WIN32
		if(open("/dev/tty", O_RDONLY) != STDIN_FILENO) {
			fprintf(stderr, "%s: Cannot open terminal as stdin\n", argv[0]);
			return 1;
		}
//#endif
#endif
	} else {
		fprintf(stderr, "%s: Cannot read from a terminal\n", argv[0]);
		return 1;
	}

	if(read_file(fp, use_pipe, flags) < 0) {
		perror(argv[0]);
		r = 1;
	}
#if defined _WIN32 && !defined _WIN32_WNT_NATIVE
	if(use_pipe) {
		// Since our ToolBox may be used as a library, restore the original file
		if(close(STDIN_FILENO) < 0) {
			perror("close");
			return 1;
		}
		if(dup(fd) != STDIN_FILENO) {
			fprintf(stderr, "%s: Cannot restore original stdin\n", argv[0]);
			return 1;
		}
		close(fd);
	}
#endif
	fclose(fp);

	return r;
}
