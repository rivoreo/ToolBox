/*	toolbox
	Copyright 2007-2015 PC GO Ld.
	Copyright 2015 libdll.so

	This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PROGRAM_NAME "libdll.so ToolBox"
#define VERSION "1.3"

static void help(void);
int main(int, char **);

static int toolbox_main(int argc, char **argv) {
	if(argc > 1) {
		if(strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0 ||
		strcmp(argv[1], "-l") == 0 || strcmp(argv[1], "--list") == 0) {
			help();
			return 0;
		}
		if(strcmp(argv[1], "--version") == 0) {
			puts(PROGRAM_NAME);
			puts("Version " VERSION);
			puts("Copyright 2007-2015 PC GO Ld.");
			puts("Copyright 2015 libdll.so");
			puts("This is free software; you are free to change and redistribute it;\n"
				"see the source for copying conditions.");
			puts("There is NO warranty; not even for MERCHANTABILITY or FITNESS FOR A\n"
				"PARTICULAR PURPOSE.");
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
	{ NULL, NULL }
};

static void help() {
	int i;
	puts(PROGRAM_NAME " " VERSION "\nCopyright 2015 libdll.so\n\n"
		"Usage: \n	toolbox <tool> [<tool-arguments>]\n	<tool> [<tool-arguments>]\n\nList of tools:");
	for(i = 2; tools[i].name; i++) printf("	%s\n", tools[i].name);
	putchar('\n');
}

int main(int argc, char **argv) {
	const char *name = argv[0];
	int i;

	if((argc > 1) && (argv[1][0] == '@')) {
		name = argv[1] + 1;
		argc--;
		argv++;
	} else {
		const char *command = strrchr(argv[0], '/');
		if(command) name = command + 1;
	}

	for(i = 0; tools[i].name; i++) {
		if(strcmp(tools[i].name, name) == 0) {
			return tools[i].func(argc, argv);
		}
	}

	printf("%s: no such tool\n", argv[0]);
	return -1;
}
