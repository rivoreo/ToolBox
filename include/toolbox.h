/*	libtoolbox
	Copyright 2015 libdll.so
 
	This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.
 
	This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#ifndef _TOOLBOX_H
#define _TOOLBOX_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __APPLE__
#define _SHARED
#endif
#define TOOL(name) int name##_main(int, char **);
#include <tools.h>
#undef TOOL

#ifdef __cplusplus
}
#endif

#endif
