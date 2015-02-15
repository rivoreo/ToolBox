#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <selinux/selinux.h>

int chcon_main(int argc, char **argv) {
	int rc, i;

	if (argc < 3) {
		fprintf(stderr, "Usage:  %s <context> <path> [...]\n", argv[0]);
		return -1;
	}

	for (i = 2; i < argc; i++) {
		rc = setfilecon(argv[i], argv[1]);
		if (rc < 0) {
			fprintf(stderr, "%s:  Could not label %s with %s:  %s\n",
					argv[0], argv[i], argv[1], strerror(errno));
			return 1;
		}
	}
	return 0;
}
