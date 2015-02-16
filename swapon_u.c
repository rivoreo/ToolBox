#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
//#include <asm/page.h>
#include <sys/swap.h>
#include <string.h>
#include <errno.h>

/* XXX These need to be obtained from kernel headers. See b/9336527 */
#define SWAP_FLAG_PREFER        0x8000
#define SWAP_FLAG_PRIO_MASK     0x7fff
#define SWAP_FLAG_PRIO_SHIFT    0
#define SWAP_FLAG_DISCARD       0x10000

void usage(const char *name) {
	fprintf(stderr, "Usage: %s [-p prio] <filename>\n"
			"	prio must be between 0 and %d\n", name, SWAP_FLAG_PRIO_MASK);
}

int parse_prio(const char *prio_str) {
	unsigned long int p = strtoul(prio_str, NULL, 10);
	return (p > SWAP_FLAG_PRIO_MASK)? -1 : (int)p;
}

int swapon_main(int argc, char **argv) {
	int flags = 0;
	int prio;
	opterr = 0;

	while(1) {
		int c = getopt(argc, argv, "hp:");
		if(c == -1) break;
		switch(c) {
			case 'p':
				if(!optarg) {
					usage(argv[0]);
					return -EINVAL;
				}
				prio = parse_prio(optarg);
				flags |= SWAP_FLAG_PREFER;
				flags |= (prio << SWAP_FLAG_PRIO_SHIFT) & SWAP_FLAG_PRIO_MASK;
				break;
			case 'h':
				usage(argv[0]);
				return 0;
			case '?':
				fprintf(stderr, "unknown option: '-%c'\n", optopt);
				return -EINVAL;
		}
	}

	if(optind != argc - 1) {
		usage(argv[0]);
		return -EINVAL;
	}

	if(swapon(argv[argc - 1], flags) < 0) {
		int e = errno;
		fprintf(stderr, "swapon failed for %s, %s\n", argv[argc - 1], strerror(e));
		return -e;
	}

	return 0;
}

