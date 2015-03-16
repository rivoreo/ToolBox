#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <termios.h>
#include <errno.h>

static struct winsize winsz;
static struct termios old, new;

static int use_tty, use_pipe;
static int page_col, page_row;
static char filename[1024];

/* Initialize new terminal i/o settings */
static void initTermios(int echo) 
{
	tcgetattr(0, &old); /* grab old terminal i/o settings */
	new = old; /* make new settings same as old settings */
	new.c_iflag &= ~ICRNL; /* Translate carriage return to newline on input (unless IGNCR is set) */
	new.c_lflag &= ~ICANON; /* disable buffered i/o */
	new.c_lflag &= echo ? ECHO : ~ECHO; /* set echo mode */
	tcsetattr(0, TCSANOW, &new); /* use these new terminal i/o settings now */
}

/* Restore old terminal i/o settings */
static void resetTermios(void) 
{
	tcsetattr(0, TCSANOW, &old);
}

/* Read 1 character - echo defines echo mode */
static char getch_(int echo) 
{
	char ch;
	initTermios(echo);
	ch = getchar();
	resetTermios();
	return ch;
}

/* Read 1 character without echo */
static char getch(void) 
{
	return getch_(0);
}

/* Read 1 character with echo */
static char getche(void) 
{
	return getch_(1);
}


static int usage(char *name) {
	fprintf(stdout, "Usage:\n"
				"%s [options] <file>...\n", name);
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
		if((fp = fopen("/dev/tty","r")) == NULL) {
			perror("Error open TTY");
			return errno;
		}

		int line_num = i -1;
		int l = 0;
		for(i-- ;i > 0; i--) {
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
		for(i=0;i<=line_num;i++) {
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
	use_tty = isatty(STDOUT_FILENO);
	int use_pipe = !isatty(STDIN_FILENO);


	if(use_tty) {
		/*
		 * Get windows size 
		 *
		 * struct winsize {
		 *	unsigned short ws_row;
		 *	unsigned short ws_col;
		 *	unsigned short ws_xpixel;
		 *	unsigned short ws_ypixel;
		 *};
		 */
		if(ioctl(STDOUT_FILENO, TIOCGWINSZ, &winsz) == -1) {
			return errno;
		}

		page_row = winsz.ws_row;
		page_col = winsz.ws_col;

	} else {
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
	}else if(argc == 1 && !use_pipe) {
		usage(argv[0]);
		return 1;
	} else if(use_pipe) {
		if((fp = fdopen(STDIN_FILENO,"r")) == NULL) {
			perror("Error open STDIN_FILENO");
			return errno;
		}
	}


	read_file(fp, use_pipe);

	return 0;
}

