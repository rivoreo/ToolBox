#include <stdio.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <errno.h>

static struct winsize winsz;
static int use_tty;
static int page_col, page_row;
static int usage(char *name) {
	fprintf(stdout, "Usage:\n"
				"%s [options] <file>...\n", name);
	return 1;
}
static int read_file(FILE *fp) {
	char line[page_col];
	while(fgets(line, page_col, fp) != NULL) {
		if(page_row != 0) {
			fputs(line,stdout);
			page_row--;
		} else {
			//Test
			return 3;
		}

	}
	return 0;
}
int more_main(int argc, char *argv[]) {
	use_tty = isatty(STDOUT_FILENO);
	/* If not a pipe */
	//if(isatty(STDIN_FILENO)) {
	//	usage(argv[0]);
	//	return 1;
	//}

	if(use_tty) {
		/* Get windows size 
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
	FILE *fp;
	if((fp = fdopen(STDIN_FILENO,"r")) == NULL) {
		fprintf(stderr,"open file err\n");
		return errno;
	}
	read_file(fp);
	return 0;
}

