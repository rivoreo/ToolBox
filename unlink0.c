//#include "rm_u.c"
#include <stdio.h>
#include <errno.h>
#include <unistd.h>

void welcome(){
	printf("unlink - toolbox\nCopyright 2007-2013 PC GO Ld.\nRemove existing files.\n\n");
	fprintf(stderr, "Usage: unlink"
		" <target...>\n");
}

int main(int argc, char *argv[]){
	int i;
	if(argc == 1 || !strcmp(argv[1], "-h") || !strcmp(argv[1], "--help")){
		welcome();
		return -1;
	}
	for(i = 1; i < argc; i++){
		int ret = unlink(argv[i]);
		if (ret < 0) {
			fprintf(stderr, "unlink failed for %s\n", argv[i]);
			return 1;
		}
	}
	return 0;
}
