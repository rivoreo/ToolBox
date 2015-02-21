/*	reboot - toolbox
	Copyright 2015 libdll.so

	This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#include <unistd.h>
#include <string.h>
#include <stdio.h>

#ifdef _WIN32
#define RB_AUTOBOOT 0x01234567
#define RB_HALT_SYSTEM 0xcdef0123
#include <errno.h>
#include <windows.h>
#ifdef _WIN32_WCE
#ifdef _USE_KIOCTL
#if 0
#include <pkfuncs.h>
#else
#include <winioctl.h>
#define IOCTL_HAL_REBOOT CTL_CODE(FILE_DEVICE_HAL, 15, METHOD_BUFFERED, FILE_ANY_ACCESS)
extern int WINAPI KernelIoControl(unsigned long int, void *, unsigned long int, void *, unsigned long int, unsigned long int *);
#endif
#else
#include <pm.h>
#endif
#else
#ifdef _WIN32_WNT_NATIVE
#include <nt.h>
#else
#ifndef SE_SHUTDOWN_PRIVILEGE
#define SE_SHUTDOWN_PRIVILEGE (19)
#endif
#endif
static int enable_shutdown_privilege() {
	void *token = NULL;
	TOKEN_PRIVILEGES privilege;
	memset(&privilege, 0, sizeof privilege);
	privilege.PrivilegeCount = 1;
	privilege.Privileges[0].Luid.LowPart = SE_SHUTDOWN_PRIVILEGE;
	privilege.Privileges[0].Luid.HighPart = 0;
	privilege.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
#ifdef _WIN32_WNT_NATIVE
	long int status = NtOpenProcessToken((void *)-1, TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &token);
	if(status < 0) {
		__set_errno_from_ntstatus(status);
		return -1;
	}
	status = NtAdjustPrivilegesToken(token, 0, &privilege, sizeof privilege, NULL, NULL);
	NtClose(token);
	if(status < 0) {
		__set_errno_from_ntstatus(status);
		return -1;
	}
	return 0;
#else
	//LUID id;
	//memset(&id, 0, sizeof id);
	if(!OpenProcessToken((void *)-1, TOKEN_ADJUST_PRIVILEGES, &token)) return -1;
	//if(!LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &id)) return -1;
	int ok = AdjustTokenPrivileges(token, 0, &privilege, sizeof privilege, NULL, NULL);
	CloseHandle(token);
	return ok ? 0 : 1;
#endif
}
#endif		/* _WIN32_WCE */

static int reboot(int flags) {
#ifdef _WIN32_WCE
	if(flags != RB_AUTOBOOT) {
		errno = EINVAL;
		return -1;
	}
#ifdef _USE_KIOCTL
	if(!KernelIoControl(IOCTL_HAL_REBOOT, NULL, 0, NULL, 0, NULL)) return -1;
#else
	unsigned long int e = SetSystemPowerState(NULL, POWER_STATE_RESET, POWER_FORCE);
	if(e) {
		SetLastError(e);
		return -1;
	}
#endif
#else
	if(enable_shutdown_privilege() < 0) return -1;
#ifdef _WIN32_WNT_NATIVE
	SHUTDOWN_ACTION action;
	switch(flags) {
		case RB_AUTOBOOT:
			action = ShutdownReboot;
			break;
		case RB_HALT_SYSTEM:
			action = ShutdownNoReboot;
			break;
		default:
			errno = EINVAL;
			return -1;
	}
	long int status = NtShutdownSystem(action);
	if(status < 0) {
		__set_errno_from_ntstatus(status);
		return -1;
	}
#else
#ifdef _USE_EWX
	unsigned int wflags = EWX_FORCE;
	switch(flags) {
		case RB_AUTOBOOT:
			flags |= EWX_REBOOT;
			break;
		case RB_HALT_SYSTEM:
			flags |= EWX_SHUTDOWN;
			break;
		default:
			errno = EINVAL;
			return -1;
	}
	if(!ExitWindowsEx(wflags, 0)) {
		return -1;
	}
#else
	if(!InitiateSystemShutdownW(NULL, NULL, 0, 1, flags & EWX_REBOOT)) return -1;
#endif
#endif		/* _WIN32_WNT_NATIVE */
#endif		/* _WIN32_WCE */
	while(1) sleep(10000);
}
#else
#include <sys/reboot.h>
#endif

int reboot_main(int argc, char **argv) {
	int flags = RB_AUTOBOOT;
#ifndef _WIN32
	if(argc < 2 || strcmp(argv[1], "-n")) sync();
#ifdef RB_NOSYNC
	else flags |= RB_NOSYNC;
#endif
#endif
	if(reboot(flags) < 0) {
		perror("reboot");
		return 1;
	}

	// Never reached?
	return 0;
}

