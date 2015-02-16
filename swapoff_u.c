#include <stdio.h>
#include <unistd.h>
//#include <asm/page.h>
#include <sys/swap.h>
#include <string.h>
#include <errno.h>

int swapoff_main(int argc, char **argv) {
	if (argc != 2) {
		fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
		return -EINVAL;
	}

	if(swapoff(argv[1]) < 0) {
		int e = errno;
		fprintf(stderr, "swapoff failed for %s, %s\n", argv[1], strerror(e));
		return -e;
	}

	return 0;
}
