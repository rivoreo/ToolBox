#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <errno.h>
#if defined __GLIBC__ || defined _WIN32
#include <sys/statfs.h>
#else
#include <sys/param.h>
#include <sys/mount.h>
#endif

static int ok = EXIT_SUCCESS;

static void printsize(long long int n) {
	char unit = 'K';
	long long int t;

	n *= 10;

	if(n > 1024 * 1024 * 10) {
		n /= 1024;
		unit = 'M';
	}

	if(n > 1024 * 1024 * 10) {
		n /= 1024;
		unit = 'G';
	}

	t = (n + 512) / 1024;
#if defined _WIN32 && !defined _WIN32_WNT_NATIVE
	printf("%4lu.%1lu%ci", (unsigned long int)(t / 10), (unsigned long int)(t % 10), unit);
#else
	printf("%4lld.%1lld%ci", t / 10, t % 10, unit);
#endif
}

static void sdf(const struct statfs *st, const char *s, int always) {
	if(st->f_blocks == 0 && !always) return;
#if defined __GNU__ || defined __linux__ || (defined _WIN32 && !defined _WIN32_WNT_NATIVE)
	printf("%-20s  ", s);
#else
	printf("%-20s  ", st->f_mntfromname);
#endif
	printsize((long long)st->f_blocks * (long long)st->f_bsize);
	printf("  ");
	printsize((long long)(st->f_blocks - (long long)st->f_bfree) * st->f_bsize);
	printf("  ");
	printsize((long long)st->f_bfree * (long long)st->f_bsize);
	printf("   %d\n", (int)st->f_bsize);
}

static void df(const char *s, int always) {
	struct statfs st;

	if(statfs(s, &st) < 0) {
		//fprintf(stderr, "%s: %s\n", s, strerror(errno));
		perror(s);
		ok = EXIT_FAILURE;
	} else {
		sdf(&st, s, always);
	}
}

int main(int argc, char *argv[]) {
	int all = 0;
	if(argc > 1 && (strcmp(argv[1], "-a") == 0 || strcmp(argv[1], "--all") == 0)) {
		all = 1;
		argc--;
		argv++;
	}
	if(argc > 1 && (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--human-readable") == 0)) {
		argc--;
		argv++;
	}
#ifdef _WIN32
	if(argc == 1) {
		fprintf(stderr, "df: You need to specify at least one path\n");
		return -1;
	}
#endif
	puts(
#if defined __GNU__ || defined __linux__ || (defined _WIN32 && !defined _WIN32_WNT_NATIVE)
	//"Mounted on"
	"File system"
#else
	//"Filesystem"
	"Device     "
#endif
	"               Size      Used      Free   Block size");
	if(argc == 1) {
#ifndef _WIN32
#if defined __GNU__ || defined __linux__
		char s[2000];
		FILE *f = fopen("/proc/mounts", "r");

		if(!f) {
			perror("/proc/mounts");
			return 1;
		}

		while(fgets(s, 2000, f)) {
			char *c, *e = s;

			for (c = s; *c; c++) {
				if (*c == ' ') {
					e = c + 1;
					break;
				}
			}

			for (c = e; *c; c++) {
				if (*c == ' ') {
					*c = 0;
					break;
				}
			}

			df(e, all);
		}

		fclose(f);
#else
		int i;
		//struct statfs buffer[200];
		int len = getfsstat(NULL, 0, MNT_NOWAIT);
		if(len < 0) {
			perror("getstatfs");
			return 1;
		}
		struct statfs buffer[len];
		if(getfsstat(buffer, len * sizeof(struct statfs), MNT_NOWAIT) < 0) {
			perror("getstatfs");
			return 1;
		}
		for(i = 0; i < len; i++) {
			sdf(buffer + i, NULL, all);
		}
#endif
#endif
	} else {
		int i;
		for(i = 1; i < argc; i++) {
			df(argv[i], 1);
		}
	}

	return ok;
}
