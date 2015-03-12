#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <errno.h>

static struct winsize winsz;
static int use_tty;
static int page_col, page_row;
static char filename[1024];
static int usage(char *name) {
	fprintf(stdout, "Usage:\n"
				"%s [options] <file>...\n", name);
	return 1;
}
static int read_file(FILE *fp) {
	char line[page_col];
	while(fgets(line, page_col, fp) != NULL) {
		if(page_row != 1) {
			fflush(stdout);
			fprintf(stdout,"%s",line);
			page_row--;
		} else {		
			return 3;
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
		
		if((fp = fdopen(STDIN_FILENO,"r")) == NULL) {
			perror("Error open STDIN_FILENO");
			return errno;
		}
	}


	read_file(fp);

	return 0;
}

