#include <stdio.h>
#include <stdlib.h>

#undef environ

int main(int argc, char **argv, char **environ) {
	if(argc == 1) while(*environ) {
		puts(*environ++);
	} else {
		int i;
		for(i=1; i<argc; i++) {
			char *v = getenv(argv[i]);
			if(v) puts(v);
		}
	}
	return 0;
}
