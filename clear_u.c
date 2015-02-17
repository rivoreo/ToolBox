/*	clear - toolbox
	Copyright 2015 libdll.so

	This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#include <unistd.h>

#define CLEAR_SCREEN "\e[H\e[J"

int clear_main() {
	size_t len = sizeof CLEAR_SCREEN - 1;
	int s = write(STDOUT_FILENO, CLEAR_SCREEN, len);
	return s == len ? 0 : 1;
}
