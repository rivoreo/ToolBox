#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <sys/stat.h>
#include <string.h>

static int usage()
{
    fprintf(stderr,"ln [-s] <target> <name>\n");
    return -1;
}

int ln_main(int argc, char *argv[])
{
	struct stat st;
	char new_name[PATH_MAX + 1];
	int symbolic = 0;
	int ret;
	int i;

	for(i=0; i<argc; i++) if(strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--symbolic") == 0) {
		int j;
		symbolic = 1;
		for(j=i; j>0; j--) argv[j] = argv[j - 1];
		argc--;
		argv++;
	}
	if(argc < 3) return usage();

	size_t old_path_len = strlen(argv[1]);
	size_t len = strlen(argv[2]);
	if(len > PATH_MAX) {
		fprintf(stderr, "new name too long\n");
		return 1;
	}
	memcpy(new_name, argv[2], len + 1);
	if(old_path_len > 1 && stat(argv[2], &st) == 0 && S_ISDIR(st.st_mode)) {
		if(argv[1][old_path_len - 1] == '/') argv[1][old_path_len - 1] = 0;
		const char *old_file_name = strrchr(argv[1], '/');
		if(old_file_name) old_file_name++;
		else old_file_name = argv[1];
		size_t old_file_name_len = strlen(old_file_name);
		if(new_name[len - 1] != '/') new_name[len++] = '/';		// Didn't need to add the '\0'
		if(len + old_file_name_len > PATH_MAX) {
			fprintf(stderr, "new name too long\n");
			return 1;
		}
		memcpy(new_name + len, old_file_name, old_file_name_len + 1);
	}

	ret = (symbolic ? symlink : link)(argv[1], new_name);
	if(ret < 0) fprintf(stderr, "link failed, %s\n", strerror(errno));
	return ret;
}
