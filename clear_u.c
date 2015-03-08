/*	clear - toolbox
	Copyright 2015 libdll.so

	This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#ifdef _WIN32
#include <windows.h>
#include <winioctl.h>
#ifndef IOCTL_CONSOLE_CLS
#define IOCTL_CONSOLE_CLS CTL_CODE(FILE_DEVICE_CONSOLE, 5, METHOD_BUFFERED, FILE_ANY_ACCESS)
#endif
#ifndef FILE_DEVICE_CONSOLE
#define FILE_DEVICE_CONSOLE 0x00000102
#endif
#ifndef STDOUT_FILENO
#ifdef _WIN32_WCE
#include <stdio.h>
#define STDOUT_FILENO fileno(stdout)
#else
#define STDOUT_FILENO ((int)GetStdHandle(STD_OUTPUT_HANDLE) ? : -1)
#endif
#endif
#else
#include <unistd.h>
#define CLEAR_SCREEN "\e[H\e[J"
#endif

int clear_main() {
#ifdef _WIN32
	unsigned long int rsize;
	return DeviceIoControl((void *)STDOUT_FILENO, IOCTL_CONSOLE_CLS, NULL, 0, NULL, 0, &rsize, NULL) ? 0 : 1;
#else
	size_t len = sizeof CLEAR_SCREEN - 1;
	return write(STDOUT_FILENO, CLEAR_SCREEN, len) == len ? 0 : 1;
#endif
}
