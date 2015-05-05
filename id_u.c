#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include <string.h>

static void print_uid(uid_t uid) {
	struct passwd *pw = getpwuid(uid);
/*
	if(pw) {
		printf("%d(%s)", uid, pw->pw_name);
	} else {
		printf("%d",uid);
	}
*/
	printf("%u", (unsigned int)uid);
	if(pw) printf("(%s)", pw->pw_name);
}

static void print_gid(gid_t gid) {
	struct group *gr = getgrgid(gid);
/*
	if(gr) {
		printf("%d(%s)", gid, gr->gr_name);
	} else {
		printf("%d",gid);
	}
*/
	printf("%u", (unsigned int)gid);
	if(gr) printf("(%s)", gr->gr_name);
}

int id_main(int argc, char **argv) {
	gid_t list[64];
	int n, max;
	//char *secctx;

	if(argc > 1) {
		if(strcmp(argv[1], "-u") == 0 || strcmp(argv[1], "--user") == 0) {
			printf("%u\n", (unsigned int)geteuid());
			return 0;
		}
		if(strcmp(argv[1], "-r") == 0 || strcmp(argv[1], "--real") == 0) {
			printf("%u\n", (unsigned int)getuid());
			return 0;
		}
		if(strcmp(argv[1], "-g") == 0 || strcmp(argv[1], "--group") == 0) {
			printf("%u\n", (unsigned int)getegid());
			return 0;
		}
		if(strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
			fprintf(stderr, "Usage: %s [<option>]\n\n"
					"Options:\n"
					"	-u, --user	Print current user ID\n"
					"	-g, --group	Print current group ID\n"
					"	-r, --real	Print current real user ID"
					" instead of effective ID\n\n", argv[0]);
			return 0;
		}
	}
		

	max = getgroups(64, list);
	if(max < 0) max = 0;

	printf("uid=");
	print_uid(getuid());
	printf(" gid=");
	print_gid(getgid());
	if(max) {
		printf(" groups=");
		print_gid(list[0]);
		for(n = 1; n < max; n++) {
			putchar(',');
			print_gid(list[n]);
		}
	}
	putchar('\n');
	return 0;
}
