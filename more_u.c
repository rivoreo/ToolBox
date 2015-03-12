#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <errno.h>

static struct winsize winsz;
static struct termios old, new;

static int use_tty;
static int page_col, page_row;
static char filename[1024];
static char end_line_text[] = "--More--";

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
			break;

	}
	return 0;
}

static int read_file(FILE *fp) {
	char line[page_col];
	while(fgets(line, page_col, fp) != NULL) {
		if(page_row != 1) {
			fprintf(stdout,"%s",line);
			//fflush(stdout);
			page_row--;
		} else {
			/* Save current cursor position */
			//printf("\x1b[7");
			fprintf(stdout, "%s", end_line_text);
			read_more();
		}

	}
	return 0;
}


int more_main(int argc, char *argv[]) {
	use_tty = isatty(STDOUT_FILENO);

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
	}
	
	/* If not a pipe */
	if(!isatty(STDIN_FILENO)) {
		
		if((fp = fopen(STDIN_FILENO,"r")) == NULL) {
			perror("Error open STDIN_FILENO");
			return errno;
		}
	}


	read_file(fp);

	return 0;
}

