#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <sys/stat.h>

static void usage()
{
	fprintf(stderr,
		"Usage: mkdir"
#if defined _WIN32 && !defined _WIN32_WNT_NATIVE
		".exe"
#endif
		" [<option>] <target> [...]\n");
	fprintf(stderr,"    --help           display usage and exit\n");
	fprintf(stderr,"    -p, --parents    create parent directories as needed\n");
	//return -1;
}

int main(int argc, char *argv[])
{
    //int symbolic = 0;
    int ret;
/*
    if(argc < 2 || strcmp(argv[1], "--help") == 0) {
        return usage();
    }

    int recursive = (strcmp(argv[1], "-p") == 0 ||
                     strcmp(argv[1], "--parents") == 0) ? 1 : 0;

    if(recursive && argc < 3) {
        // -p specified without a path
        return usage();
    }

    if(recursive) {
        argc--;
        argv++;
    }
*/
	int recursive = 0;
	if(argc > 1 && (strcmp(argv[1], "-p") == 0 || strcmp(argv[1], "--parents") == 0)) {
		recursive = 1;
		argc--;
		argv++;
	}

	if(argc < 2 || strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
		usage();
		return -1;
	}

    char currpath[PATH_MAX], *pathpiece;
    struct stat st;

    while(argc > 1) {
        argc--;
        argv++;
        if(recursive) {
            // reset path
            //strcpy(currpath, "");
            *currpath = 0;
            // create the pieces of the path along the way
            pathpiece = strtok(argv[0], "/");
            if(argv[0][0] == '/') {
                // prepend / if needed
                strcat(currpath, "/");
            }
            while(pathpiece != NULL) {
                if(strlen(currpath) + strlen(pathpiece) + 2/*NUL and slash*/ > PATH_MAX) {
                    fprintf(stderr, "Invalid path specified: too long\n");
                    return 1;
                }
                strcat(currpath, pathpiece);
                strcat(currpath, "/");
                if(stat(currpath, &st) < 0) {
#if defined _WIN32 && !defined _WIN32_WNT_NATIVE
			ret = mkdir(currpath);
#else
			ret = mkdir(currpath, 0777);
#endif
                    if(ret < 0) {
//#ifdef _WIN32_WCE
//				fprintf(stderr, "mkdir failed for %s\n%d\n", currpath, errno);
//#else
				fprintf(stderr, "mkdir failed for %s, %s\n", currpath, strerror(errno));
//#endif
				return ret;
                    }
                }
                pathpiece = strtok(NULL, "/");
            }
        } else {
#if defined _WIN32 && !defined _WIN32_WNT_NATIVE
		ret = mkdir(argv[0]);
#else
		ret = mkdir(argv[0], 0777);
#endif
            if(ret < 0) {
//#ifdef _WIN32_WCE
//			fprintf(stderr, "mkdir failed for %s\n%d\n", argv[0], errno);
//#else
			fprintf(stderr, "mkdir failed for %s, %s\n", argv[0], strerror(errno));
//#endif
			return ret;
            }
        }
    }

    return 0;
}
