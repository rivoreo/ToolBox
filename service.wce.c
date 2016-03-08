/*	service - toolbox (for Windows CE)
	Copyright 2015 libdll.so

	This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#include <windows.h>
#include <service.h>
#include <winioctl.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

static const char *get_status_string(unsigned long int status) {
	switch(status) {
		case SERVICE_STATE_OFF:
			return "stop";
		case SERVICE_STATE_ON:
			return "start/running";
		case SERVICE_STATE_STARTING_UP:
			return "starting";
		case SERVICE_STATE_SHUTTING_DOWN:
			return "stoping";
		case SERVICE_STATE_UNLOADING:
			return "unloading";
		case SERVICE_STATE_UNINITIALIZED:
			return "uninitialized";
		case SERVICE_STATE_UNKNOWN:
		default:
			return "unknown";
	}
}

int service_main(int argc, char **argv) {
	if(argc < 2 || strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
		fprintf(stderr, "Usage: %s <service> [<action>]\n"
			"Actions:\n"
			"	start\n"
			"	stop\n"
			"	status\n\n", argv[0]);
		return -1;
	}

	size_t len = strlen(argv[1]) + 1;
	wchar_t service[len];
	if(mbstowcs(service, argv[1], len) == (size_t)-1) {
		fprintf(stderr, "%s: Invalid service name\n", argv[0]);
		return 1;
	}

	if(!argv[2]) {
		if(GetServiceHandle(service, NULL, NULL) == (void *)-1) {
			perror(argv[1]);
			return 1;
		}
		return 0;
	} else if(strcmp(argv[2], "start") == 0) {
		if(!ActivateService(service, 0)) {
			perror(argv[1]);
			return 1;
		}
		return 0;
	} else if(strcmp(argv[2], "stop") == 0) {
		fprintf(stderr, "%s: stop: %s\n", argv[0], strerror(ENOSYS));
		return 2;
	} else if(strcmp(argv[2], "status") == 0) {
		unsigned long int status;
		void *sh = GetServiceHandle(service, NULL, NULL);
		if(sh == (void *)-1) {
			perror(argv[1]);
			return 1;
		}
		DeviceIoControl(sh, IOCTL_SERVICE_STATUS, NULL, 0, &status, sizeof status, NULL, NULL);
		printf("%s is %s\n", argv[1], get_status_string(status));
		return status == SERVICE_STATE_ON ? 0 : (status == SERVICE_STATE_UNKNOWN ? 1 : 3);
	} else {
		fprintf(stderr, "%s: Unknown action '%s'\n", argv[0], argv[2]);
		return 1;
	}
}
