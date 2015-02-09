#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void help(void);
int main(int, char **);
int i;

static int toolbox_main(int argc, char **argv) {
	if(argc > 1) {
		if(strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0 ||
		strcmp(argv[1], "-l") == 0 || strcmp(argv[1], "--list") == 0) {
			help();
			return 0;
		}

		// "toolbox foo ..." is equivalent to "foo ..."
		return main(argc - 1, argv + 1);
	}

	puts("Toolbox!");
	return 0;
}

#define TOOL(name) int name##_main(int, char **);
#include "tools.h"
#undef TOOL

static struct {
	const char *name;
	int (*func)(int, char **);
} tools[] = {
	{ "toolbox", toolbox_main },
	{ "libtoolbox.so", toolbox_main },
#define TOOL(name) { #name, name##_main },
#include "tools.h"
#undef TOOL
	{ 0, 0 }
};

static void help() {
	puts("libdll.so ToolBox 1.3\nCopyright 2007-2015 PC GO Ld.\n\nUsage: \n	toolbox <tool> [<tool-arguments>]\n	<tool> [<tool-arguments>]\n\nList of tools:");
	for(i = 2; tools[i].name; i++) printf("	%s\n", tools[i].name);
	putchar('\n');
}

int main(int argc, char **argv)
{
	char *name = argv[0];

	if((argc > 1) && (argv[1][0] == '@')) {
		name = argv[1] + 1;
		argc--;
		argv++;
	} else {
		char *command = strrchr(argv[0], '/');
		if(command) name = command + 1;
	}

	for(i = 0; tools[i].name; i++) {
		if(!strcmp(tools[i].name, name)) {
			return tools[i].func(argc, argv);
		}
	}

	printf("%s: no such tool\n", argv[0]);
	return -1;
}
