/*	ls - toolbox
	Copyright 2007-2015 PC GO Ld.
	Copyright 2015 libdll.so

	This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#define _FILE_OFFSET_BITS 64
#define _ALL_SOURCE
#define __USE_ISOC99
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <math.h>

#if !defined __linux__ && !defined _NO_SELINUX
#define _NO_SELINUX
#endif

#ifdef _NO_SELINUX
#ifdef _WIN32
#define SHORT_OPTIONS "AadFhilpRs"
#else
#define SHORT_OPTIONS "AadFhilnpRs"
#endif
#else
//#define SHORT_OPTIONS "lsRdZAaFpnih"
#define SHORT_OPTIONS "AadFhilnpRsZ"
#endif

#ifndef _WIN32
#include <pwd.h>
#include <grp.h>
#endif
#include <stdbool.h>

#ifdef __linux__
//#include <linux/kdev_t.h>
#ifndef _NO_SELINUX
#include <selinux/selinux.h>
#endif
#endif

#include <limits.h>

#ifdef _WINDOWSNT_NATIVE
// The nativelibc didn't implement snprintf yet
#if 0
#define snprintf(D, S, ...) sprintf(D, __VA_ARGS__)
#else
// Using ntdll's _snprintf
extern int _snprintf(char *, size_t, const char *, ...);
#define snprintf _snprintf
#endif
#ifndef major
#define major(d) (0)
#endif
#ifndef minor
#define minor(d) (0)
#endif

#else

#ifdef _WIN32
#include <windows.h>
#define lstat stat
#ifndef S_IXGRP
#define S_IXGRP 00010
#endif
#ifndef S_IXOTH
#define S_IXOTH 00001
#endif
#ifndef _WIN32_WCE
#undef STDOUT_FILENO
#if 0
#define STDOUT_FILENO 1
#define isatty _isatty
#else
#define STDOUT_FILENO ((int)GetStdHandle(STD_OUTPUT_HANDLE) ? : -1)
#undef isatty
#endif
#endif		/* !_WIN32_WCE */
#else
#if !defined major || !defined minor
#include <sys/mkdev.h>
#endif
#endif		/* _WIN32 */
#endif		/* _WINDOWSNT_NATIVE */

#ifndef NAN
//#if __GNUC_PREREQ (3,3)
//#ifdef __sun
////#define NAN (__builtin_nan)
//#define NAN (nan(NULL))
//#else
#define NAN (__builtin_nanf(""))
//#endif
//#endif
#endif

#ifdef __sun
#undef NAN
#define NAN (__builtin_nanf(""))
//#if isnan == __builtin_isnan
//#include <ieeefp.h>
#undef isnan
//#define isnan isnand
//#endif
#include <nan.h>
#define isnan NaN
#endif

/* Test COLOR */
static bool is_color = false;

#if 0
// The secand argument of this macro should be a char array not a pointer
#define COLOR_PRINT(_color, _str, _input_str) \
	do {											\
		if(is_color) {									\
			snprintf((_str), sizeof(_str), "\e[%sm%s\e[0m", _color, (_input_str)); 	\
		} else {									\
			snprintf((_str), sizeof(_str), "%s", (_input_str));			\
		}										\
	} while(0)

#define COLOR_RESET "0"

#define COLOR_BLACK "0;30"
#define COLOR_RED "0;31"
#define COLOR_GREEN "0;32"
#define COLOR_YELLOW "0;33"
#define COLOR_BLUE "0;34"
#define COLOR_PURPLE "0;35"
#define COLOR_CRAN "0;36"
#define COLOR_GRAY "0;37"

#define COLOR_BOLD_BLACK "1;30"
#define COLOR_BOLD_RED "1;31"
#define COLOR_BOLD_GREEN "1;32"
#define COLOR_BOLD_YELLOW "1;33"
#define COLOR_BOLD_BLUE "1;34"
#define COLOR_BOLD_PURPLE "1;35"
#define COLOR_BOLD_CRAN "1;36"
#define COLOR_BOLD_WHITE "1;37"
#endif

//#define TERMINAL_CONTROL_START "\e["
//#define TERMINAL_CONTROL_END "m"

enum color {
	NO_COLOR,

	//COLOR_RESET,

	COLOR_BLACK,
	COLOR_RED,
	COLOR_GREEN,
	COLOR_YELLOW,
	COLOR_BLUE,
	COLOR_PURPLE,
	COLOR_CRAN,
	COLOR_GRAY,

	COLOR_BOLD,
	COLOR_BOLD_BLACK,
	COLOR_BOLD_RED,
	COLOR_BOLD_GREEN,
	COLOR_BOLD_YELLOW,
	COLOR_BOLD_BLUE,
	COLOR_BOLD_PURPLE,
	COLOR_BOLD_CRAN,
	COLOR_BOLD_WHITE,

	COLOR_BACKGROUND_BLACK,
	COLOR_BACKGROUND_RED,
	COLOR_BACKGROUND_GREEN,
	COLOR_BACKGROUND_YELLOW,
	COLOR_BACKGROUND_BLUE,
	COLOR_BACKGROUND_PURPLE,
	COLOR_BACKGROUND_CRAN,
	COLOR_BACKGROUND_GRAY
};

#if !defined _WIN32 || defined _WINDOWSNT_NATIVE
static const char *terminal_colors[] = {
	//[COLOR_RESET] = "0",
	[COLOR_BLACK] = "30",
	[COLOR_RED] = "31",
	[COLOR_GREEN] = "32",
	[COLOR_YELLOW] = "33",
	[COLOR_BLUE] = "34",
	[COLOR_PURPLE] = "35",
	[COLOR_CRAN] = "36",
	[COLOR_GRAY] = "37",
	[COLOR_BOLD] = "1",
	[COLOR_BOLD_BLACK] = "1;30",
	[COLOR_BOLD_RED] = "1;31",
	[COLOR_BOLD_GREEN] = "1;32",
	[COLOR_BOLD_YELLOW] = "1;33",
	[COLOR_BOLD_BLUE] = "1;34",
	[COLOR_BOLD_PURPLE] = "1;35",
	[COLOR_BOLD_CRAN] = "1;36",
	[COLOR_BOLD_WHITE] = "1;37",
	[COLOR_BACKGROUND_BLACK] = "40",
	[COLOR_BACKGROUND_RED] = "41",
	[COLOR_BACKGROUND_GREEN] = "42",
	[COLOR_BACKGROUND_YELLOW] = "43",
	[COLOR_BACKGROUND_BLUE] = "44",
	[COLOR_BACKGROUND_PURPLE] = "45",
	[COLOR_BACKGROUND_CRAN] = "46",
	[COLOR_BACKGROUND_GRAY] = "47"
};
#elif defined _WIN32_WCE && defined _USE_LIBPORT && _USE_LIBPORT == 2
static int (*_SetConsoleTextAttribute)(void *, unsigned short int);
#define SetConsoleTextAttribute _SetConsoleTextAttribute
#define GetProcAddress GetProcAddressA
#endif

static int printf_color(int color, const char *format, ...) {
	//fprintf(stderr, "function: printf_color(%d, %p<%s>, ...)\n", color, format, format);
	char buffer[4096];
	int i = 0;
	int r = 0;
	va_list ap;
	va_start(ap, format);
#if !defined _WIN32_WCE || defined _USE_LIBPORT
	int high_color = color >> 16;
	color &= 0xffff;
#if defined _WIN32 && !defined _WINDOWSNT_NATIVE
	int j = 0;
#ifdef _WIN32_WCE
	//void *fh = (void *)fileno(stdout);
	void *fh = (void *)STDOUT_FILENO;
#else
	void *fh = GetStdHandle(STD_OUTPUT_HANDLE);
#endif
	unsigned short int get_console_attribute(int t) {
		switch(t) {
			case COLOR_BLACK:
				return 0;
			case COLOR_BOLD_BLACK:
				return FOREGROUND_INTENSITY;
			case COLOR_RED:
				return FOREGROUND_RED;
			case COLOR_BOLD_RED:
				return FOREGROUND_RED | FOREGROUND_INTENSITY;
			case COLOR_GREEN:
				return FOREGROUND_GREEN;
			case COLOR_BOLD_GREEN:
				return FOREGROUND_GREEN | FOREGROUND_INTENSITY;
			case COLOR_BLUE:
				return FOREGROUND_BLUE;
			case COLOR_BOLD_BLUE:
				return FOREGROUND_BLUE | FOREGROUND_INTENSITY;
			case COLOR_PURPLE:
				return FOREGROUND_RED | FOREGROUND_BLUE;
			case COLOR_BOLD_PURPLE:
				return FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
			case COLOR_YELLOW:
				return FOREGROUND_RED | FOREGROUND_GREEN;
			case COLOR_BOLD_YELLOW:
				return FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
			case COLOR_CRAN:
				return FOREGROUND_BLUE | FOREGROUND_GREEN;
			case COLOR_BOLD_CRAN:
				return FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
			case COLOR_BOLD_WHITE:
				return FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
			//case COLOR_GRAY:
			//	return FOREGROUND_INTENSITY;
			case COLOR_BACKGROUND_BLACK:
				return 0;
			case COLOR_BACKGROUND_RED:
				return BACKGROUND_RED;
			case COLOR_BACKGROUND_GREEN:
				return BACKGROUND_GREEN;
			case COLOR_BACKGROUND_BLUE:
				return BACKGROUND_BLUE;
			case COLOR_BACKGROUND_PURPLE:
				return BACKGROUND_RED | BACKGROUND_BLUE;
			case COLOR_BACKGROUND_YELLOW:
				return BACKGROUND_RED | BACKGROUND_GREEN;
			case COLOR_BACKGROUND_CRAN:
				return BACKGROUND_BLUE | BACKGROUND_GREEN;
			case COLOR_BACKGROUND_GRAY:
				return BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE;
			default:
				fprintf(stderr, "toolbox warning: ls: printf_color: get_console_attribute: color %d not supported\n", color);
			case NO_COLOR:
				return FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
		}
	}
	unsigned short int attrib = get_console_attribute(color);
	if(high_color) attrib |= get_console_attribute(high_color);
#endif
#endif
	while(*format && i < sizeof buffer) {
		if(*format == '%') {
			if(is_color && color != NO_COLOR) switch(format[1]) {
				case 'V':
#if !defined _WIN32_WCE || defined _USE_LIBPORT
#if defined _WIN32 && !defined _WINDOWSNT_NATIVE
					buffer[i] = 0;
					r += vprintf(buffer, ap);
					i = 0;
					while(j) {
						(void)va_arg(ap, int);
						j--;
					}
					SetConsoleTextAttribute(fh, attrib);
#else
					i += sprintf(buffer + i, "\e[%sm", terminal_colors[color]);
					if(high_color) {
						i += sprintf(buffer + i, "\e[%sm", terminal_colors[high_color]);
					}
#endif
#endif
					format += 2;
					continue;
				case 'v':
#if !defined _WIN32_WCE || defined _USE_LIBPORT
#if defined _WIN32 && !defined _WINDOWSNT_NATIVE
					buffer[i] = 0;
					r += vprintf(buffer, ap);
					i = 0;
					while(j) {
						(void)va_arg(ap, int);
						j--;
					}
					SetConsoleTextAttribute(fh, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
#else
					memcpy(buffer + i, "\e[0m", 4);
					i += 4;
#endif
#endif
					format += 2;
					continue;
#if defined _WIN32 && !defined _WINDOWSNT_NATIVE && (!defined _WIN32_WCE || defined _USE_LIBPORT)
				default:
					j++;
#endif
			} else if(format[1] == 'V' || format[1] == 'v') {
				format += 2;
				continue;
			}
		}
		buffer[i++] = *format++;
	}
	buffer[i] = 0;

	r = vprintf(buffer, ap);
	va_end(ap);
	return r;
}


/* # Terminal Color
 * # Reset
 * Color_Off='\e[0m'       # Text Reset
 *
 * # Regular Colors
 * Black='\e[0;30m'        # Black
 * Red='\e[0;31m'          # Red
 * Green='\e[0;32m'        # Green
 * Yellow='\e[0;33m'       # Yellow
 * Blue='\e[0;34m'         # Blue
 * Purple='\e[0;35m'       # Purple
 * Cyan='\e[0;36m'         # Cyan
 * White='\e[0;37m'        # White
 *
 * # Bold
 * BBlack='\e[1;30m'       # Black
 * BRed='\e[1;31m'         # Red
 * BGreen='\e[1;32m'       # Green
 * BYellow='\e[1;33m'      # Yellow
 * BBlue='\e[1;34m'        # Blue
 * BPurple='\e[1;35m'      # Purple
 * BCyan='\e[1;36m'        # Cyan
 * BWhite='\e[1;37m'       # White
 *
 * # Underline
 * UBlack='\e[4;30m'       # Black
 * URed='\e[4;31m'         # Red
 * UGreen='\e[4;32m'       # Green
 * UYellow='\e[4;33m'      # Yellow
 * UBlue='\e[4;34m'        # Blue
 * UPurple='\e[4;35m'      # Purple
 * UCyan='\e[4;36m'        # Cyan
 * UWhite='\e[4;37m'       # White
 *
 * # Background
 * On_Black='\e[40m'       # Black
 * On_Red='\e[41m'         # Red
 * On_Green='\e[42m'       # Green
 * On_Yellow='\e[43m'      # Yellow
 * On_Blue='\e[44m'        # Blue
 * On_Purple='\e[45m'      # Purple
 * On_Cyan='\e[46m'        # Cyan
 * On_White='\e[47m'       # White
 *
 * # High Intensity
 * IBlack='\e[0;90m'       # Black
 * IRed='\e[0;91m'         # Red
 * IGreen='\e[0;92m'       # Green
 * IYellow='\e[0;93m'      # Yellow
 * IBlue='\e[0;94m'        # Blue
 * IPurple='\e[0;95m'      # Purple
 * ICyan='\e[0;96m'        # Cyan
 * IWhite='\e[0;97m'       # White
 *
 * # Bold High Intensity
 * BIBlack='\e[1;90m'      # Black
 * BIRed='\e[1;91m'        # Red
 * BIGreen='\e[1;92m'      # Green
 * BIYellow='\e[1;93m'     # Yellow
 * BIBlue='\e[1;94m'       # Blue
 * BIPurple='\e[1;95m'     # Purple
 * BICyan='\e[1;96m'       # Cyan
 * BIWhite='\e[1;97m'      # White
 *
 * # High Intensity backgrounds
 * On_IBlack='\e[0;100m'   # Black
 * On_IRed='\e[0;101m'     # Red
 * On_IGreen='\e[0;102m'   # Green
 * On_IYellow='\e[0;103m'  # Yellow
 * On_IBlue='\e[0;104m'    # Blue
 * On_IPurple='\e[0;105m'  # Purple
* On_ICyan='\e[0;106m'    # Cyan
* On_IWhite='\e[0;107m'   # White
*/


// dynamic arrays
typedef struct {
	int count;
	int capacity;
	void **items;
} dynarray_t;

#define DYNARRAY_INITIALIZER { 0, 0, NULL }

/*
static void dynarray_init(dynarray_t *a)
{
	a->count = a->capacity = 0;
	a->items = NULL;
}
*/

static void dynarray_reserve_more(dynarray_t *a, int count) {
	int old_cap = a->capacity;
	int new_cap = old_cap;
	const int max_cap = INT_MAX / sizeof(void *);
	void **new_items;
	int new_count = a->count + count;

	if(count <= 0) return;

	if(count > max_cap - a->count) abort();

	new_count = a->count + count;

	while (new_cap < new_count) {
		old_cap = new_cap;
		new_cap += (new_cap >> 2) + 4;
		if (new_cap < old_cap || new_cap > max_cap) {
			new_cap = max_cap;
		}
	}
	new_items = realloc(a->items, new_cap * sizeof(void *));
	if(!new_items) abort();

	a->items = new_items;
	a->capacity = new_cap;
}

static void dynarray_append(dynarray_t *a, void *item) {
	if(a->count >= a->capacity) dynarray_reserve_more(a, 1);
	a->items[a->count++] = item;
}

static void dynarray_done(dynarray_t *a) {
	free(a->items);
	a->items = NULL;
	a->count = a->capacity = 0;
}

#define DYNARRAY_FOREACH_TYPE(_array,_item_type,_item,_stmnt) \
	do { \
		int _nn_##__LINE__ = 0; \
		for(;_nn_##__LINE__ < (_array)->count; ++ _nn_##__LINE__) { \
			_item_type _item = (_item_type)(_array)->items[_nn_##__LINE__]; \
			_stmnt; \
		} \
	} while(0)

#define DYNARRAY_FOREACH(_array,_item,_stmnt) \
	DYNARRAY_FOREACH_TYPE(_array,void *,_item,_stmnt)


// string arrays

typedef dynarray_t strlist_t;

#define STRLIST_INITIALIZER DYNARRAY_INITIALIZER

#define STRLIST_FOREACH(_list,_string,_stmnt) \
	DYNARRAY_FOREACH_TYPE(_list,char *,_string,_stmnt)

static void strlist_append_b(strlist_t *list, const void *str, size_t slen) {
	char *copy = malloc(slen + 1);
	memcpy(copy, str, slen);
	copy[slen] = '\0';
	dynarray_append(list, copy);
}

static void strlist_append_dup(strlist_t *list, const char *str) {
	strlist_append_b(list, str, strlen(str));
}

static void strlist_done(strlist_t *list) {
	STRLIST_FOREACH(list, string, free(string));
	dynarray_done(list);
}

static int strlist_compare_strings(const void *a, const void *b) {
	const char *sa = *(const char **)a;
	const char *sb = *(const char **)b;
	return strcasecmp(sa, sb);
}

static void strlist_sort(strlist_t *list) {
	if(list->count > 0) {
		qsort(list->items,
			(size_t)list->count,
			sizeof(void *),
			strlist_compare_strings);
	}
}

// bits for flags argument
#define LIST_LONG		(1 << 0)
#define LIST_ALL		(1 << 1)
#define LIST_RECURSIVE		(1 << 2)
#define LIST_DIRECTORIES	(1 << 3)
#define LIST_SIZE		(1 << 4)
#define LIST_CLASSIFY		(1 << 6)
#define LIST_ALL_ALMOST		(1 << 7)
#define LIST_MACLABEL		(1 << 8)
#define LIST_PATH_SLASH		(1 << 9)
#define LIST_NUMERIC_ID		(1 << 10)
#define LIST_INODE		(1 << 11)
#define LIST_HUMAN_READABLE	(1 << 12)
#define LIST_FILE_TYPE		(1 << 13)
#define MULTI_FILES		(1 << 15)

// fwd
static int listpath(const char *name, int flags);

static char mode2kind(unsigned int mode) {
	switch(mode & S_IFMT) {
#if !defined _WIN32 || defined _WINDOWSNT_NATIVE
		case S_IFSOCK: return 's';
		case S_IFLNK: return 'l';
		case S_IFBLK: return 'b';
		case S_IFCHR: return 'c';
#endif
		case S_IFIFO: return 'p';
		case S_IFREG: return '-';
		case S_IFDIR: return 'd';

		default: return '?';
	}
}

static void mode2str(unsigned int mode, char *out) {
	*out++ = mode2kind(mode);

	*out++ = (mode & 0400) ? 'r' : '-';
	*out++ = (mode & 0200) ? 'w' : '-';
	if(mode & 04000) {
		*out++ = (mode & 0100) ? 's' : 'S';
	} else {
		*out++ = (mode & 0100) ? 'x' : '-';
	}
	*out++ = (mode & 040) ? 'r' : '-';
	*out++ = (mode & 020) ? 'w' : '-';
	if(mode & 02000) {
		*out++ = (mode & 010) ? 's' : 'S';
	} else {
		*out++ = (mode & 010) ? 'x' : '-';
	}
	*out++ = (mode & 04) ? 'r' : '-';
	*out++ = (mode & 02) ? 'w' : '-';
	if(mode & 01000) {
		*out++ = (mode & 01) ? 't' : 'T';
	} else {
		*out++ = (mode & 01) ? 'x' : '-';
	}
	*out = 0;
}

#ifndef _WIN32
static void user2str(unsigned int uid, char *out) {
	struct passwd *pw = getpwuid(uid);
	if(pw) {
		strcpy(out, pw->pw_name);
	} else {
		sprintf(out, "%u", uid);
	}
}

static void group2str(unsigned int gid, char *out) {
	struct group *gr = getgrgid(gid);
	if(gr) {
		strcpy(out, gr->gr_name);
	} else {
		sprintf(out, "%u", gid);
	}
}
#endif

static int get_file_color_by_mode(mode_t mode, const char *pathname) {
	switch(mode & S_IFMT) {
		case S_IFDIR:
			return COLOR_BOLD_BLUE;
#if !defined _WIN32 || defined _WINDOWSNT_NATIVE
		case S_IFSOCK:
			return COLOR_BOLD_PURPLE;
		case S_IFBLK:
		case S_IFCHR:
			return COLOR_BOLD_YELLOW | (COLOR_BACKGROUND_BLACK << 16);
		case S_IFLNK:
			if(pathname) {
				struct stat st;
				if(stat(pathname, &st) < 0) return COLOR_BOLD_RED;
			}
			return COLOR_BOLD_CRAN;
#endif
		case S_IFIFO:
			return COLOR_YELLOW | (COLOR_BACKGROUND_BLACK << 16);
			//return COLOR_PURPLE;
		case S_IFREG:
#ifndef _WIN32
			if(mode & S_ISUID) return COLOR_BACKGROUND_RED | (COLOR_GRAY << 16);
			if(mode & S_ISGID) return COLOR_BACKGROUND_YELLOW | (COLOR_BLACK << 16);
#endif
			if(mode & (S_IXUSR | S_IXGRP | S_IXOTH)) return COLOR_BOLD_GREEN;
			// Fall
		default:
			return NO_COLOR;
	}
}

static int get_file_color(const char *pathname) {
	struct stat st;
	if(lstat(pathname, &st) < 0) return NO_COLOR;
	return get_file_color_by_mode(st.st_mode, pathname);
}

static double human_readable_d(double n, int *unit) {
	if(!unit) return NAN;
	switch(*unit) {
		case 0:
			if(n < 1024) return n;
			n /= 1024;
			*unit = 'K';
		case 'K':
			if(n < 1024) return n;
			n /= 1024;
			*unit = 'M';
		case 'M':
			if(n < 1024) return n;
			n /= 1024;
			*unit = 'G';
		case 'G':
			if(n < 1024) return n;
			n /= 1024;
			*unit = 'T';
			return n;
		default:
			return NAN;
	}
}

static int show_total_size(const char *dirname, DIR *d, int flags) {
	struct dirent *de;
	char tmp[1024];
	struct stat s;
	int sum = 0;

	/* run through the directory and sum up the file block sizes */
	while((de = readdir(d))) {
		if(!(flags & LIST_ALL) && (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0)) continue;
		if(de->d_name[0] == '.' && !(flags & LIST_ALL) && !(flags & LIST_ALL_ALMOST)) continue;
		const char *slash = "/";
		if(dirname[strlen(dirname) - 1] == '/') slash = "";
		snprintf(tmp, sizeof(tmp), "%s%s%s", dirname, slash, de->d_name);

		if(lstat(tmp, &s) < 0) {
			fprintf(stderr, "ls: stat '%s' failed: %s\n", tmp, strerror(errno));
			rewinddir(d);
			return -1;
		}
#if defined _WIN32 && !defined _WINDOWSNT_NATIVE
		sum += s.st_size / 1024;		// XXX
#else
		sum += s.st_blocks / 2;
#endif
	}

	if(flags & LIST_HUMAN_READABLE) {
		int unit = 'K';
		double n = human_readable_d(sum, &unit);
		if(isnan(n)) return -1;
		printf("total %.1f%ci\n", n, unit);
	} else printf("total %d\n", sum);
	rewinddir(d);
	return 0;
}

static void show_inode(const struct stat *st) {
	if(!st) return;
#if defined _WIN32 && !defined _WINDOWSNT_NATIVE
	printf("%8lu ", (unsigned long int)st->st_ino);
#else
	printf("%8llu ", (unsigned long long int)st->st_ino);
#endif
}

static int listfile_other(const char *path, const char *filename, const struct stat *st, int flags) {
	struct stat s;
	if(!st) {
		if(lstat(path, &s) < 0) {
			fprintf(stderr, "ls: lstat '%s' failed: %s\n", path, strerror(errno));
			return -1;
		}
		st = &s;
	}

	if(flags & LIST_INODE) show_inode(st);

	/* blocks are 512 bytes, we want output to be KB */
	if((flags & LIST_SIZE) != 0) {
#if defined _WIN32 && !defined _WINDOWSNT_NATIVE
		long int size = st->st_size / 1024;		// XXX
#else
		long int size = st->st_blocks / 2;
#endif
		if(size && (flags & LIST_HUMAN_READABLE)) {
			int unit = 'K';
			double n = human_readable_d(size, &unit);
			if(isnan(n)) printf("? ");
			else printf("%.1f%ci ", n, unit);
		} else printf("%ld ", size);
	}

	char filetype = mode2kind(st->st_mode);
	if((flags & LIST_CLASSIFY) != 0) {
		if(filetype != 'l') {
			printf("%c ", filetype);
		} else {
			struct stat link_dest;
			if(stat(path, &link_dest) == 0) {
				printf("l%c ", mode2kind(link_dest.st_mode));
			} else {
				fprintf(stderr, "ls: stat '%s' failed: %s\n", path, strerror(errno));
				printf("l? ");
			}
		}
	}

	const char *suffix = "";
	if((flags & LIST_PATH_SLASH) && filetype == 'd') suffix = "/";
#if !defined _WIN32 || defined _WINDOWSNT_NATIVE
	else if(flags & LIST_FILE_TYPE) switch(filetype) {
		case 'p':
			suffix = "|";
			break;
		case 'l':
			suffix = "@";
			break;
		case 's':
			suffix = "=";
			break;
	}
#endif

	if(is_color) printf_color(get_file_color_by_mode(st->st_mode, path), "%V%s%v%s\n", filename, suffix);
	else printf("%s%s\n", filename, suffix);

	return 0;
}

static int listfile_long(const char *path, int flags) {
	//fprintf(stderr, "function: listfile_long(%p<%s>, %d)\n", path, path, flags);
	struct stat s;
	char date[32];
	char mode[16];
	char user[16];
	char group[16];
	const char *name;
	char size[16];

	if((flags & LIST_DIRECTORIES) || !(name = strrchr(path, '/')) || !*++name) name = path;

	if(lstat(path, &s) < 0) {
		int e = errno;
		//perror(path);
		fprintf(stderr, "ls: lstat '%s' failed: %s\n", path, strerror(e));
		if(e != ENOENT) {
			printf("??????????   ? ?      ?             ? \?\?\?\?-\?\?-\?\? \?\?:\?\? %s\n", name);
		}
		return -1;
	}

	if(flags & LIST_INODE) show_inode(&s);

	mode2str(s.st_mode, mode);
#ifndef _WIN32
	if(flags & LIST_NUMERIC_ID) {
#endif
//#ifdef __INTERIX
//		sprintf(user, "%lu", s.st_uid);
//		sprintf(group, "%lu", s.st_gid);
//#else
		sprintf(user, "%u", (unsigned int)s.st_uid);
		sprintf(group, "%u", (unsigned int)s.st_gid);
//#endif
#ifndef _WIN32
	} else {
		user2str(s.st_uid, user);
		group2str(s.st_gid, group);
	}
#endif

	if(flags & LIST_HUMAN_READABLE) {
		int unit = 0;
		double n = human_readable_d(s.st_size, &unit);
		if(isnan(n)) *size = 0;
		else {
			if(unit) sprintf(size, "%.1f%ci", n, unit);
			else sprintf(size, "%u", (unsigned int)s.st_size);
		}
	} else {
#if defined _WIN32 && !defined _WINDOWSNT_NATIVE
		sprintf(size, "%8ld", s.st_size);
#else
		sprintf(size, "%8lld", (long long int)s.st_size);
#endif
	}

	if(!strftime(date, 32, "%Y-%m-%d %H:%M", localtime(&s.st_mtime))) {
		strcpy(date, "\?\?\?\?-\?\?-\?\? \?\?:\?\?");
	}
	date[31] = 0;

	// 12345678901234567890123456789012345678901234567890123456789012345678901234567890
	// MMMMMMMMMM LLL UUUUUU GGGGGG XXXXXXXX YYYY-MM-DD HH:MM NAME [-> SLINKTARGET]

	switch(s.st_mode & S_IFMT) {
		case S_IFDIR:
			printf_color(COLOR_BOLD_BLUE, "%s %3u %-6s %-6s %8s %s %V%s%v%s\n",
				mode, (unsigned int)s.st_nlink, user, group, size, date,
				name, flags & LIST_PATH_SLASH ? "/" : "");
			break;
#if !defined _WIN32 || defined _WINDOWSNT_NATIVE
		case S_IFSOCK:
			//COLOR_PRINT(COLOR_BOLD_PURPLE, file, name);
			printf_color(COLOR_BOLD_PURPLE, "%s %3u %-6s %-6s          %s %V%s%v%s\n",
				mode, (unsigned int)s.st_nlink, user, group, date, name,
				flags & LIST_FILE_TYPE ? "=" : "");
			break;
		case S_IFBLK:
		case S_IFCHR:
			//COLOR_PRINT(COLOR_BOLD_YELLOW, file, name);
			printf_color(COLOR_BOLD_YELLOW | (COLOR_BACKGROUND_BLACK << 16),
				"%s %3u %-6s %-6s %3d, %3d %s %V%s%v\n",
				mode, (unsigned int)s.st_nlink, user, group,
				(int)major(s.st_rdev), (int)minor(s.st_rdev), date, name);
			break;
#endif
		case S_IFIFO:
			printf_color(COLOR_YELLOW | (COLOR_BACKGROUND_BLACK << 16),
				"%s %3u %-6s %-6s          %s %V%s%v%s\n",
				mode, (unsigned int)s.st_nlink, user, group, date, name,
				flags & LIST_FILE_TYPE ? "|" : "");
			break;
		case S_IFREG:
			if(is_color) {
				int color;
#ifndef _WIN32
				if(s.st_mode & S_ISUID) color = COLOR_BACKGROUND_RED | (COLOR_GRAY << 16);
				else if(s.st_mode & S_ISGID) color = COLOR_BACKGROUND_YELLOW | (COLOR_BLACK << 16);
				else
#endif
				if(s.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH)) color = COLOR_BOLD_GREEN;
				else goto reg_nocolor;
				printf_color(color, "%s %3u %-6s %-6s %8s %s %V%s%v\n",
					mode, (unsigned int)s.st_nlink, user, group, size, date, name);
			} else {
		reg_nocolor:
				printf("%s %3u %-6s %-6s %8s %s %s\n",
					mode, (unsigned int)s.st_nlink, user, group, size, date, name);
			}
			break;
#if !defined _WIN32 || defined _WINDOWSNT_NATIVE
		case S_IFLNK: {
			char linkto[256];
			int len;
			int color = COLOR_BOLD_CRAN;
			struct stat st_linkto;
			const char *suffix = "";

			len = readlink(path, linkto, 256);
			if(len < 0) color = COLOR_BOLD_RED;
			else {
				if(len > 255) {
					linkto[252] = '.';
					linkto[253] = '.';
					linkto[254] = '.';
					linkto[255] = 0;
				} else {
					linkto[len] = 0;
				}
				if(stat(path, &st_linkto) < 0) color = COLOR_BOLD_RED;
			}

			printf_color(color, "%s %3u %-6s %-6s %8s %s %V%s%v",
				mode, (unsigned int)s.st_nlink, user, group, size, date, name);
			if(len < 0) {
				putchar('\n');
				break;
			}
			if(color == COLOR_BOLD_CRAN) {
				color = get_file_color_by_mode(st_linkto.st_mode, NULL);
				if((flags & LIST_PATH_SLASH) && S_ISDIR(st_linkto.st_mode)) suffix = "/";
				else if(flags & LIST_FILE_TYPE) switch(st_linkto.st_mode & S_IFMT) {
					case S_IFIFO:
						suffix = "|";
						break;
					case S_IFSOCK:
						suffix = "=";
						break;
				}
			}
			printf_color(color, " -> %V%s%v%s\n", linkto, suffix);
			break;
		}
#endif
		default:
			printf("%s %3u %-6s %-6s          %s %s\n",
				mode, (unsigned int)s.st_nlink, user, group, date, name);
	}
	return 0;
}

#ifndef _NO_SELINUX
static int listfile_maclabel(const char *path, int flags) {
	//fprintf(stderr, "function: listfile_maclabel(%p<%s>, 0x%x)\n", path, path, flags);
	struct stat s;
	char mode[16];
	char user[16];
	char group[16];
	char *maclabel = NULL;
	const char *name;
	char size[16];

	if((flags & LIST_DIRECTORIES) || !(name = strrchr(path, '/')) || !*++name) name = path;

	if(lstat(path, &s) < 0) {
		perror(path);
		return -1;
	}

	if(flags & LIST_INODE) show_inode(&s);

	const char *suffix = ((flags & LIST_PATH_SLASH) && S_ISDIR(s.st_mode)) ? "/" : "";
	if(!*suffix && (flags & LIST_FILE_TYPE)) switch(s.st_mode & S_IFMT) {
		case S_IFIFO:
			suffix = "|";
			break;
		case S_IFLNK:
			suffix = "@";
			break;
		case S_IFSOCK:
			suffix = "=";
			break;
	}
	lgetfilecon(path, &maclabel);

	if(!(flags & LIST_LONG)) {
		printf("%s ", maclabel ? : "?");
		if(flags & (LIST_SIZE | LIST_CLASSIFY)) listfile_other(path, name, &s, flags & ~LIST_INODE);
		else printf_color(get_file_color_by_mode(s.st_mode, path), "%V%s%v%s\n", name, suffix);
		free(maclabel);
		return 0;
	}

	mode2str(s.st_mode, mode);
#ifndef _WIN32
	if(flags & LIST_NUMERIC_ID) {
#endif
		sprintf(user, "%u", (unsigned int)s.st_uid);
		sprintf(group, "%u", (unsigned int)s.st_gid);
#ifndef _WIN32
	} else {
		user2str(s.st_uid, user);
		group2str(s.st_gid, group);
	}
#endif

	if((flags & LIST_SIZE) && (flags & LIST_HUMAN_READABLE)) {
		int unit = 0;
		double n = human_readable_d(s.st_size, &unit);
		if(isnan(n)) *size = 0;
		else {
			if(unit) sprintf(size, "%.1f%ci", n, unit);
			else sprintf(size, "%u", (unsigned int)s.st_size);
		}
	} else sprintf(size, "%lu", (unsigned long int)s.st_size);

	switch(s.st_mode & S_IFMT) {
		case S_IFLNK: {
			char linkto[256];
			ssize_t len;
			int color = COLOR_BOLD_CRAN;
			struct stat st_linkto;

			len = readlink(path, linkto, sizeof(linkto));
			if(len < 0) color = COLOR_BOLD_RED;
			else {
				if((size_t)len > sizeof(linkto)-1) {
					linkto[sizeof(linkto)-4] = '.';
					linkto[sizeof(linkto)-3] = '.';
					linkto[sizeof(linkto)-2] = '.';
					linkto[sizeof(linkto)-1] = 0;
				} else {
					linkto[len] = 0;
				}
				if(stat(path, &st_linkto) < 0) color = COLOR_BOLD_RED;
			}

			if(flags & LIST_SIZE) {
				printf("%s %8u %8s", mode, (unsigned int)s.st_nlink, size);
			} else {
				printf("%s %-8s %-8s", mode, user, group);
			}
			printf_color(color, " %s %V%s%v", maclabel ? : "?", name);
			if(len < 0) {
				putchar('\n');
				break;
			}
			if(color == COLOR_BOLD_CRAN) {
				color = get_file_color_by_mode(st_linkto.st_mode, NULL);
				if((flags & LIST_PATH_SLASH) && S_ISDIR(st_linkto.st_mode)) suffix = "/";
				else if(flags & LIST_FILE_TYPE) switch(st_linkto.st_mode & S_IFMT) {
					case S_IFIFO:
						suffix = "|";
						break;
					case S_IFSOCK:
						suffix = "=";
						break;
				}
			} else suffix = 0;
			printf_color(color, " -> %V%s%v%s\n", linkto, suffix);
			break;
		}
		default:
			if(flags & LIST_SIZE) {
				printf("%s %8u %8s", mode, (unsigned int)s.st_nlink, size);
			} else {
				printf("%s %-8s %-8s", mode, user, group);
			}
			printf_color(get_file_color_by_mode(s.st_mode, NULL),
				" %s %V%s%v%c\n", maclabel ? : "?", name, suffix);
			break;
	}

	free(maclabel);

	return 0;
}
#endif

static int listfile(const char *dirname, const char *filename, int flags) {
	//struct stat s;
	//char file[4096];
	char tmp[4096];
	const char *name;
	const char *pathname = filename;

	if(dirname != NULL) {
		const char *slash = "/";
		if(dirname[strlen(dirname) - 1] == '/') slash = "";
		snprintf(tmp, sizeof(tmp), "%s%s%s", dirname, slash, filename);
		pathname = tmp;
	} else {
		pathname = filename;
	}

	if((flags & (LIST_LONG | LIST_SIZE | LIST_CLASSIFY | LIST_PATH_SLASH | LIST_MACLABEL | LIST_INODE | LIST_FILE_TYPE)) == 0) {
		/* name is anything after the final '/', or the whole path if none*/
		if((flags & LIST_DIRECTORIES) || !(name = strrchr(pathname, '/'))) name = pathname;
		else name++;

		if(is_color) printf_color(get_file_color(pathname), "%V%s%v\n", name);
		else puts(name);

		return 0;
	}

#ifndef _NO_SELINUX
	if(flags & LIST_MACLABEL) {
		return listfile_maclabel(pathname, flags);
	} else
#endif
	if ((flags & LIST_LONG) != 0) {
		return listfile_long(pathname, flags);
	} else {
		return listfile_other(pathname, filename, NULL, flags);
	}
}

static int listdir(const char *name, int flags) {
	char tmp[4096];
	DIR *d;
	struct dirent *de;
	strlist_t files = STRLIST_INITIALIZER;
	int list_all = flags & LIST_ALL;

	d = opendir(name);
	if(!d) {
		fprintf(stderr, "ls: opendir '%s' failed, %s\n", name, strerror(errno));
		return -1;
	}

	if((flags & LIST_SIZE) || (flags & LIST_LONG)) {
		show_total_size(name, d, flags);
	}

	while((de = readdir(d))) {
		if(!list_all && (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0)) continue;
		if(de->d_name[0] == '.' && !list_all && !(flags & LIST_ALL_ALMOST)) continue;

		strlist_append_dup(&files, de->d_name);
	}

	strlist_sort(&files);
	STRLIST_FOREACH(&files, filename, listfile(name, filename, flags));
	strlist_done(&files);

	if(flags & LIST_RECURSIVE) {
		strlist_t subdirs = STRLIST_INITIALIZER;

		rewinddir(d);

		while((de = readdir(d))) {
			struct stat s;
			int err;

			if(strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0) continue;
			if(de->d_name[0] == '.' && !list_all && !(flags & LIST_ALL_ALMOST)) continue;

			const char *slash = "/";
			if(name[strlen(name) - 1] == '/') slash = "";
			snprintf(tmp, sizeof(tmp), "%s%s%s", name, slash, de->d_name);

			/*
			 * If the name ends in a '/', use stat() so we treat it like a
			 * directory even if it's a symlink.
			 */
			err = (tmp[strlen(tmp)-1] == '/' ? stat : lstat)(tmp, &s);

			if(err < 0) {
				perror(tmp);
				continue;
			}

			if(S_ISDIR(s.st_mode)) {
				strlist_append_dup(&subdirs, tmp);
			}
		}
		strlist_sort(&subdirs);
		STRLIST_FOREACH(&subdirs, path, {
			printf("\n%s:\n", path);
			listdir(path, flags);
		});
		strlist_done(&subdirs);
	}

	closedir(d);
	return 0;
}

static int listpath(const char *name, int flags) {
	struct stat s;
	int err;

	/*
	 * If the name ends in a '/', use stat() so we treat it like a
	 * directory even if it's a symlink.
	 */
	err = (name[strlen(name)-1] == '/' ? stat : lstat)(name, &s);

	if(err < 0) {
		//perror(name);
		fprintf(stderr, "ls: %s: %s\n", name, strerror(errno));
		return -1;
	}

	if(!(flags & LIST_DIRECTORIES) && S_ISDIR(s.st_mode)) {
		if(flags & LIST_RECURSIVE || flags & MULTI_FILES) printf("%s:\n", name);
		int r = listdir(name, flags);
		if(flags & LIST_RECURSIVE || flags & MULTI_FILES) putchar('\n');
		return r;
	} else {
		/* yeah this calls stat() again */
		return listfile(NULL, name, flags);
	}
}

int ls_main(int argc, char **argv) {
	int flags = 0;
	if(getenv("COLORTERM")) is_color = isatty(STDOUT_FILENO);
	if(argc > 1) {
		int i;
		int err = 0;
		int end_of_options = 0;
		strlist_t files = STRLIST_INITIALIZER;

		for(i = 1; i < argc; i++) {
			if(!end_of_options && argv[i][0] == '-') {
				/* an option ? */
				const char *arg = argv[i] + 1;
				if(!*arg) goto not_an_option;
				while(arg[0]) {
					switch(arg[0]) {
						case 'l': flags |= LIST_LONG; break;
						case 's': flags |= LIST_SIZE; break;
						case 'R': flags |= LIST_RECURSIVE; break;
						case 'd': flags |= LIST_DIRECTORIES; break;
#ifndef _NO_SELINUX
						case 'Z': flags |= LIST_MACLABEL; break;
#endif
						case 'A':
							flags |= LIST_ALL_ALMOST;
							flags &= ~LIST_ALL;
							break;
						case 'a':
							flags |= LIST_ALL;
							//flags &= ~LIST_ALL_ALMOST;
							break;
						case 'F': flags |= LIST_CLASSIFY; break;
						case 'p': flags |= LIST_PATH_SLASH; break;
						case 'n': flags |= LIST_NUMERIC_ID; break;
						case 'i': flags |= LIST_INODE; break;
						case 'h': flags |= LIST_HUMAN_READABLE; break;
						case '-':
							if(!arg[1]) end_of_options = 1;
							else {
								const char *long_arg = arg + 1;
								if(strcmp(long_arg, "all") == 0) {
									flags |= LIST_ALL;
								} else if(strcmp(long_arg, "color") == 0) {
									is_color = isatty(STDOUT_FILENO);
								} else if(strncmp(long_arg, "color=", 6) == 0) {
									const char *a = long_arg + 6;
									if(!*a) {
										fprintf(stderr, "%s: You forgot to specify an argument for --color after '='\n",
											argv[0]);
										return 1;
									} else if(strcmp(a, "auto") == 0) {
										is_color = isatty(STDOUT_FILENO);
									} else if(strcmp(a, "always") == 0 || strcmp(a, "force") == 0) {
										is_color = true;
									} else if(strcmp(a, "never") == 0 || strcmp(a, "none") == 0) {
										is_color = false;
									} else {
										fprintf(stderr, "%s: Unknown argument '%s' for --color\n",
											argv[0], a);
										return 1;
									}
								} else if(strcmp(long_arg, "directory") == 0) {
									flags |= LIST_DIRECTORIES;
								} else if(strcmp(long_arg, "file-type") == 0) {
									flags |= LIST_FILE_TYPE;
									flags |= LIST_PATH_SLASH;
								} else if(strcmp(long_arg, "help") == 0) {
									fprintf(stderr, "Usage: %s [-" SHORT_OPTIONS "]"
#if !defined _WIN32_WCE || defined _USE_LIBPORT
										" [--color[=<when>]]"
#endif
										" [--file-type]"
#ifdef _WIN32_WCE
										" <file>"
#endif
										" [<file>] [...]\n", argv[0]);
									return 0;
								} else if(strcmp(long_arg, "human-readable") == 0) {
									flags |= LIST_HUMAN_READABLE;
								} else if(strcmp(long_arg, "inode") == 0) {
									flags |= LIST_INODE;
								} else if(strcmp(long_arg, "numeric-uid-gid") == 0) {
									flags |= LIST_NUMERIC_ID;
								} else {
									fprintf(stderr, "%s: Unknown option '%s'. Aborting.\n", argv[0], argv[i]);
									return 1;
								}
							}
							goto out_of_inner_loop;
						default:
							fprintf(stderr, "%s: Unknown option '-%c'. Aborting.\n", argv[0], arg[0]);
							return 1;
					}
					arg++;
				}
out_of_inner_loop:		;
			} else {
not_an_option:
				/* not an option ? */
				strlist_append_dup(&files, argv[i]);
			}
		}

#if defined _WIN32_WCE && defined _USE_LIBPORT && _USE_LIBPORT == 2
		if(is_color) {
			HMODULE libport = LoadLibraryW(L"port.dll");
			if(libport) {
				*(void **)&_SetConsoleTextAttribute = GetProcAddress(libport, "SetConsoleTextAttribute");
				if(!_SetConsoleTextAttribute) is_color = false;
			} else is_color = false;
		}
#endif
		if(files.count > 0) {
			if(files.count > 1) flags |= MULTI_FILES;
			STRLIST_FOREACH(&files, path, {
				if(listpath(path, flags) < 0) {
					err = EXIT_FAILURE;
				}
			});
			strlist_done(&files);
			return err;
		}
	}

#ifdef _WIN32_WCE
	fprintf(stderr, "%s: You need to specify at least one file\n", argv[0]);
	return 1;
#else
	// list working directory if no files or directories were specified
	return -listpath(".", flags);
#endif
}
