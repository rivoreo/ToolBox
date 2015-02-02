#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <selinux/selinux.h>

int getenforce_main() {
	int rc;

	rc = is_selinux_enabled();
	if(rc <= 0) {
		printf("Disabled\n");
		return 0;
	}

	rc = security_getenforce();
	if(rc < 0) {
		fprintf(stderr, "Could not get enforcing status:  %s\n", strerror(errno));
		return 2;
	}
	puts(rc ? "Enforcing" : "Permissive");

	return 0;
}
