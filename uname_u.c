/*	uname - toolbox
	Copyright 2015-2016 Rivoreo

	This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#define _ALL_SOURCE
#include <sys/utsname.h>
#include <sys/param.h>
#ifdef BSD
#include <sys/sysctl.h>
#elif defined __sun && defined __SVR4
#include <sys/systeminfo.h>
#endif
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#ifndef SYS_NMLN
#ifdef _UTS_BUFSIZE
#define SYS_NMLN _UTS_BUFSIZE
#else
#define SYS_NMLN 64
#endif
#endif

int uname_main(int argc, char **argv) {
	struct utsname un;
	int space = 0;
	int all = 0;
	int s = 0;
	int n = 0;
	int r = 0;
	int v = 0;
	int m = 0;
	int p = 0;
	int i = 0;
	int o = 0;
	int expanded = 0;
	while(1) {
		int opt = getopt(argc, argv, "asnrvmpioX");
		if(opt == -1) break;
		switch(opt) {
			case 'a':
				all = 1;
				s = n = r = v = m = p = i = o = 1;
				break;
			case 's': s = 1; break;
			case 'n': n = 1; break;
			case 'r': r = 1; break;
			case 'v': v = 1; break;
			case 'm': m = 1; break;
			case 'p': p = 1; break;
			case 'i': i = 1; break;
			case 'o': o = 1; break;
			case 'X': expanded = 1; break;
			default:
#ifdef __ANDROID__
				fputc('\n', stderr);
#endif
				return -1;
		}
	}
	if(!s && !n && !r && !v && !m && !p && !i && !o && !expanded) s = 1;
	if(uname(&un) < 0) {
		perror("uname");
		return 1;
	}
	if(s) {
		if(space) putchar(' ');
		fputs(un.sysname, stdout);
		space = 1;
	}
	if(n) {
		if(space) putchar(' ');
		fputs(un.nodename, stdout);
		space = 1;
	}
	if(r) {
		if(space) putchar(' ');
		fputs(un.release, stdout);
		space = 1;
	}
	if(v) {
		if(space) putchar(' ');
		fputs(un.version, stdout);
		space = 1;
	}
	if(m) {
		if(space) putchar(' ');
		fputs(un.machine, stdout);
		space = 1;
	}
	if(p) {
		char buffer[SYS_NMLN];
#ifdef BSD
		int mib[] = { CTL_HW, HW_MACHINE_ARCH };
		size_t len = sizeof buffer;
		if(sysctl(mib, sizeof mib / sizeof mib[0], &buffer, &len, NULL, 0) == -1) {
#elif defined __sun && defined __SVR4
		if(sysinfo(SI_ARCHITECTURE, buffer, sizeof buffer) == -1) {
#else
		{
#endif
			if(all) *buffer = 0;
			else strcpy(buffer, "UNKNOWN");
		}
		if(*buffer) {
			if(space) putchar(' ');
			space = 1;
		}
		fputs(buffer, stdout);
	}
	if(i) {
		char buffer[SYS_NMLN];
#ifdef BSD
		size_t len = sizeof buffer;
		if(sysctlbyname("kern.ident", &buffer, &len, NULL, 0) == -1) {
#elif defined __sun && defined __SVR4
		if(sysinfo(SI_PLATFORM, buffer, sizeof buffer) == -1) {
#else
		{
#endif
			if(all) *buffer = 0;
			else strcpy(buffer, "UNKNOWN");
		}
		if(*buffer) {
			if(space) putchar(' ');
			space = 1;
		}
		fputs(buffer, stdout);
	}
	if(o) {
		// The operating system name, a GNU extension.
#ifdef __GLIBC__
 #ifdef __linux__
 #define OS_NAME "GNU/Linux"
 #elif (defined __GNU__ && defined __MACH__) || defined __gnu_hurd__
 #define OS_NAME "GNU/Hurd"
 #elif defined __FreeBSD_kernel__
 #define OS_NAME "GNU/kFreeBSD"
 #else
 #define OS_NAME "GNU"
 #endif
#elif defined __FreeBSD__
#define OS_NAME "FreeBSD"
#elif defined __NetBSD__
#define OS_NAME "NetBSD"
#elif defined __APPLE__
 #ifdef __MACH__
 #define OS_NAME "Darwin"
 #else
 #define OS_NAME "A/UX"
 #endif
#elif defined BSD
#define OS_NAME "BSD"
#elif defined __sun
 #ifdef __SVR4
 #define OS_NAME "Solaris"
 #else
 #define OS_NAME "SunOS"
 #endif
#elif defined __UCLIBC__
 #ifdef __linux__
 #define OS_NAME "uC/Linux"
 #else
 #define OS_NAME "uClibc"	/* uClibc on non-Linux? */
 #endif
#elif defined __ANDROID__
 #ifdef __linux__
 #define OS_NAME "Android/Linux"
 #else
 #define OS_NAME "Android"	/* Maybe Android been ported to kFreeBSD some day... */
 #endif
#elif defined __INTERIX
#define OS_NAME "Interix"
#elif defined __CYGWIN__
#define OS_NAME "Cygwin"
#elif defined __SVR4
#define OS_NAME "System-V"
#endif
#ifdef OS_NAME
		if(space) putchar(' ');
		fputs(OS_NAME, stdout);
#else
		if(!all) {
			if(space) putchar(' ');
			fputs("UNKNOWN", stdout);
		}
#endif
	}
	if(expanded) {
		if(space) putchar('\n');
		printf("System = %s\n"
			"Node = %s\n"
			"Release = %s\n"
			"KernelID = %s\n"
			"Machine = %s\n"
#ifdef _SC_NPROCESSORS_CONF
			"NumCPU = %d\n"
#endif
			,
			un.sysname,
			un.nodename,
			un.release,
			un.version,
			un.machine
#ifdef _SC_NPROCESSORS_CONF
			, (int)sysconf(_SC_NPROCESSORS_CONF)
#endif
			);
	}
	putchar('\n');
	return 0;
}
