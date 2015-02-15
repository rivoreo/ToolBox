#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <selinux/selinux.h>
#include <errno.h>

static int do_setsebool(char *name, const char *value) {
	SELboolean b;

	if(is_selinux_enabled() <= 0) return 0;

	b.name = name;
	if(strcmp(value, "1") == 0 || strcasecmp(value, "true") == 0 || strcasecmp(value, "on") == 0) {
		b.value = 1;
	} else if(strcmp(value, "0") == 0 || strcasecmp(value, "false") == 0 || strcasecmp(value, "off") == 0) {
		b.value = 0;
	} else {
		fprintf(stderr, "setsebool: invalid value %s\n", value);
		return -1;
	}

	if(security_set_boolean_list(1, &b, 0) < 0) {
		fprintf(stderr, "setsebool: could not set %s to %s: %s", name, value, strerror(errno));
		return -1;
	}

	return 0;
}

int main(int argc, char **argv) {
	if(argc != 3) {
		fprintf(stderr, "Usage: %s <name> <value>\n", argv[0]);
		//exit(1);
		return -1;
	}

	return do_setsebool(argv[1], argv[2]);
}
