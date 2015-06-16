#ifdef __INTERIX
#define _NO_STATFS
#include <dirent.h>
#endif

#ifdef __NetBSD__
#define _NO_STATFS
#define getfsstat getvfsstat
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#ifdef _NO_STATFS
#include <sys/statvfs.h>
#define statfs statvfs
#define f_bsize f_frsize
#else
#if defined __GLIBC__ || defined _WIN32
#include <sys/statfs.h>
#else
#include <sys/param.h>
#include <sys/mount.h>
#endif
#endif

static int ok = EXIT_SUCCESS;

static void printsize(long long n)
{
    char unit = 'K';
    long long t;

    n *= 10;

    if (n > 1024*1024*10) {
        n /= 1024;
        unit = 'M';
    }

    if (n > 1024*1024*10) {
        n /= 1024;
        unit = 'G';
    }

    t = (n + 512) / 1024;
    printf("%4lld.%1lld%ci", t/10, t%10, unit);
}

static void sdf(const struct statfs *st, const char *s, int always) {
	if(st->f_blocks == 0 && !always) return;
#if defined __GNU__ || defined __linux__ || (!defined __NetBSD__ && !defined __INTERIX && defined _NO_STATFS)
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
		fprintf(stderr, "%s: %s\n", s, strerror(errno));
		ok = EXIT_FAILURE;
	} else {
		sdf(&st, s, always);
	}
}

static void print_header() {
	puts("Filesystem                Size     Used     Free   Blksize");
}

int df_main(int argc, char *argv[]) {
	if (argc == 1) {
#if defined __GNU__ || defined __linux__
		char s[2000];
		FILE *f = fopen("/proc/mounts", "r");

		if(!f) {
			perror("/proc/mounts");
			return 1;
		}

		print_header();
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
                    *c = '\0';
                    break;
                }
            }

            df(e, 0);
        }

        fclose(f);
#elif defined __SVR4
		char s[2000];
		FILE *f = fopen("/etc/mnttab", "r");
		if(!f) {
			perror("/etc/mnttab");
			return 1;
		}

		print_header();
		while(fgets(s, sizeof s, f)) {
			char *c = s;
			while(*c) if(*c++ == '	') {
				char *e = c;
				while(*++c) if(*c == '	') {
					*c = 0;
					break;
				}
				df(e, all);
				break;
			}
		}
		fclose(f);
#elif defined __INTERIX
		DIR *d = opendir("/dev/fs");
		if(!d) {
			perror("/dev/fs");
			return 1;
		}
		print_header();
		struct dirent *de;
		while((de = readdir(d))) {
			char buffer[10] = "/dev/fs/";
			memcpy(buffer + 8, de->d_name, 2);
			df(buffer, 0);
		}
		closedir(d);
#elif !defined _NO_STATFS || defined __NetBSD__
		int len = getfsstat(NULL, 0, MNT_NOWAIT), i;
		if(len < 0) {
			perror("getfsstat");
			return 1;
		}
		struct statfs buffer[len];
		if(getfsstat(buffer, len * sizeof(struct statfs), MNT_NOWAIT) < 0) {
			perror("getfsstat");
			return 1;
		}
		print_header();
		for(i = 0; i < len; i++) sdf(buffer + i, NULL, 0);
#else
		fprintf(stderr, "df: You need to specify at least one path\n");
		return -1;
#endif
	} else {
		int i;
		print_header();
		for(i = 1; i < argc; i++) {
			df(argv[i], 1);
		}
	}

	return ok;
}
