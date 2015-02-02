#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <windows.h>
#include <nt.h>

int unlink(const char *name) {
	UNICODE_STRING name1;
	RtlCreateUnicodeStringFromAsciiz(&name1, name);
	// TODO: Convert the UNIX path name to NT style
	OBJECT_ATTRIBUTES o = { sizeof(OBJECT_ATTRIBUTES), NULL, &name1, 0, NULL, NULL };
	long int status = NtDeleteFile(&o);
	RtlFreeUnicodeString(&name1);
	printf("debug: status = 0x%lx\n", status);
	//return ((errno = ntstatus_to_errno(status)) ? -1 : 0;
	return status ? -1 : 0;
}

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
