#include <stdio.h>
#include <stdlib.h>

extern char ** environ;

int printenv_main(int argc, char **argv)
{
    if(argc == 1) {
        while(*environ) {
            puts(*environ++);
        }
    } else {
	int i;
        for(i=1; i<argc; i++) {
            char *v = getenv(argv[i]);
            if(v) puts(v);
        }
    }

    return 0;
}

