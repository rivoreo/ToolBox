#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

static int usage()
{
	fprintf(stderr,"Usage: rmdir"
#if defined _WIN32 && !defined _WIN32_WNT_NATIVE
		".exe"
#endif
		" <directory> [...]\n");
	return -1;
}

int main(int argc, char *argv[])
{
    //int symbolic = 0;
    int ret;
    if(argc < 2) return usage();

    while(argc > 1) {
        argc--;
        argv++;
        ret = rmdir(argv[0]);
        if(ret < 0) {
//#ifdef _WIN32_WCE
//		fprintf(stderr, "rmdir failed for %s, %d\n", argv[0], errno);
//#else
		fprintf(stderr, "rmdir failed for %s, %s\n", argv[0], strerror(errno));
//#endif
            return ret;
        }
    }
    
    return 0;
}
