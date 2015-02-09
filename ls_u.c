#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>

#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

#ifdef _NO_SELINUX
#define SHORT_OPTIONS "lsRdAaF"
#else
#define SHORT_OPTIONS "lsRdZAaF"
#endif

#if !defined __linux__ && !defined _NO_SELINUX
#define _NO_SELINUX
#endif

#ifndef _WIN32
#include <pwd.h>
#include <grp.h>
#endif
#include <stdbool.h>

#ifdef __linux__
#include <linux/kdev_t.h>
#ifndef _NO_SELINUX
#include <selinux/selinux.h>
#endif


#include <linux/kdev_t.h>
#elif defined __MACH__
#ifdef __APPLE__
#ifndef MAJOR
#define MAJOR(A) ((A) >> 24)
#endif
#ifndef MINOR
#define MINOR(A) ((A) & 0xff)
#endif
#elif defined __GNU__
#ifndef MAJOR
#define MAJOR(A) (0)
#endif
#ifndef MINOR
#define MINOR(A) (0)
#endif
#endif
#elif defined _WIN32_WNT_NATIVE
#ifndef MAJOR
#define MAJOR(A) (0)
#endif
#ifndef MINOR
#define MINOR(A) (0)
#endif
#endif

#include <limits.h>

#ifdef _WIN32
#ifdef _WIN32_WNT_NATIVE
// The nativelibc didn't implement snprintf yet
#if 0
#define snprintf(D, S, ...) sprintf(D, __VA_ARGS__)
#else
// Using ntdll's _snprintf
int _snprintf(char *, size_t, const char *, ...);
#define snprintf _snprintf
#endif
#else
#include <windows.h>
#define lstat stat
#endif
#endif

/* Test COLOR */
bool is_color = false;

#define COLOR_PRINT(_color,_str, _input_str) \
	do { \
		if(is_color) \
		snprintf(_str, sizeof(_str), "\e[%sm%s\e[0m", _color, _input_str); \
		else \
		snprintf(_str, sizeof(_str), "%s", _input_str); \
	} while(0) 

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

#define DYNARRAY_INITIALIZER  { 0, 0, NULL }

static void dynarray_init(dynarray_t *a)
{
	a->count = a->capacity = 0;
	a->items = NULL;
}

static void dynarray_reserve_more(dynarray_t *a, int count)
{
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
	if (new_items == NULL)
		abort();

	a->items = new_items;
	a->capacity = new_cap;
}

static void dynarray_append(dynarray_t *a, void *item)
{
	if (a->count >= a->capacity)
		dynarray_reserve_more(a, 1);

	a->items[a->count++] = item;
}

static void dynarray_done(dynarray_t *a)
{
	free(a->items);
	a->items = NULL;
	a->count = a->capacity = 0;
}

#define DYNARRAY_FOREACH_TYPE(_array,_item_type,_item,_stmnt) \
	do { \
		int _nn_##__LINE__ = 0; \
		for (;_nn_##__LINE__ < (_array)->count; ++ _nn_##__LINE__) { \
			_item_type _item = (_item_type)(_array)->items[_nn_##__LINE__]; \
			_stmnt; \
		} \
	} while (0)

#define DYNARRAY_FOREACH(_array,_item,_stmnt) \
	DYNARRAY_FOREACH_TYPE(_array,void *,_item,_stmnt)

// string arrays

typedef dynarray_t  strlist_t;

#define  STRLIST_INITIALIZER  DYNARRAY_INITIALIZER

#define  STRLIST_FOREACH(_list,_string,_stmnt) \
	DYNARRAY_FOREACH_TYPE(_list,char *,_string,_stmnt)

static void strlist_init(strlist_t *list)
{
	dynarray_init(list);
}

static void strlist_append_b(strlist_t *list, const void *str, size_t slen)
{
	char *copy = malloc(slen + 1);
	memcpy(copy, str, slen);
	copy[slen] = '\0';
	dynarray_append(list, copy);
}

static void strlist_append_dup(strlist_t *list, const char *str)
{
	strlist_append_b(list, str, strlen(str));
}

static void strlist_done(strlist_t *list)
{
	STRLIST_FOREACH(list, string, free(string));
	dynarray_done(list);
}

static int strlist_compare_strings(const void *a, const void *b)
{
	const char *sa = *(const char **)a;
	const char *sb = *(const char **)b;
	return strcmp(sa, sb);
}

static void strlist_sort( strlist_t *list )
{
	if (list->count > 0) {
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
#define MULTI_FILES		(1 << 9)

// fwd
static int listpath(const char *name, int flags);

static char mode2kind(unsigned int mode) {
	switch(mode & S_IFMT) {
#if !defined _WIN32 || defined _WIN32_WNT_NATIVE
		case S_IFSOCK: return 's';
		case S_IFLNK: return 'l';
		case S_IFBLK: return 'b';
		case S_IFCHR: return 'c';
		case S_IFIFO: return 'p';
#endif
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
static void user2str(unsigned int uid, char *out)
{
	struct passwd *pw = getpwuid(uid);
	if(pw) {
		strcpy(out, pw->pw_name);
	} else {
		sprintf(out, "%u", uid);
	}
}

static void group2str(unsigned int gid, char *out)
{
	struct group *gr = getgrgid(gid);
	if(gr) {
		strcpy(out, gr->gr_name);
	} else {
		sprintf(out, "%u", gid);
	}
}
#endif

static int show_total_size(const char *dirname, DIR *d, int flags)
{
	struct dirent *de;
	char tmp[1024];
	struct stat s;
	int sum = 0;

	/* run through the directory and sum up the file block sizes */
	while ((de = readdir(d)) != 0) {
		if(!(flags & LIST_ALL) && (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0)) continue;
		if(de->d_name[0] == '.' && !(flags & LIST_ALL) && !(flags & LIST_ALL_ALMOST)) continue;
		const char *slash = "/";
		if(dirname[strlen(dirname) - 1] == '/') slash = "";
		snprintf(tmp, sizeof(tmp), "%s%s%s", dirname, slash, de->d_name);

		if(lstat(tmp, &s) < 0) {
			fprintf(stderr, "stat failed on %s: %s\n", tmp, strerror(errno));
			rewinddir(d);
			return -1;
		}
#if defined _WIN32 && !defined _WIN32_WNT_NATIVE
		sum += s.st_size / 1024;		// XXX
#else
		sum += s.st_blocks / 2;
#endif
	}

	printf("total %d\n", sum);
	rewinddir(d);
	return 0;
}

static int listfile_size(const char *path, const char *filename, int flags)
{
	struct stat s;

	if(lstat(path, &s) < 0) {
		fprintf(stderr, "lstat '%s' failed: %s\n", path, strerror(errno));
		return -1;
	}

	/* blocks are 512 bytes, we want output to be KB */
	if ((flags & LIST_SIZE) != 0) {
#if defined _WIN32 && !defined _WIN32_WNT_NATIVE
		long int size = s.st_size / 1024;		// XXX
#else
		long int size = s.st_blocks / 2;
#endif
		printf("%ld ", size);
	}

	if ((flags & LIST_CLASSIFY) != 0) {
		char filetype = mode2kind(s.st_mode);
		if (filetype != 'l') {
			printf("%c ", filetype);
		} else {
			struct stat link_dest;
			if(stat(path, &link_dest) == 0) {
				printf("l%c ", mode2kind(link_dest.st_mode));
			} else {
				fprintf(stderr, "stat '%s' failed: %s\n", path, strerror(errno));
				printf("l? ");
			}
		}
	}

	printf("%s\n", filename);

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

	/* name is anything after the final '/', or the whole path if none*/
	//if(flags & LIST_DIRECTORIES) name = path;
	//else if((name = strrchr(path, '/')) name++;
	//else name = path;
	char file[4096];

	if((flags & LIST_DIRECTORIES) || !(name = strrchr(path, '/'))) name = path;
	else name++;

	if(lstat(path, &s) < 0) {
		int e = errno;
		//perror(path);
		fprintf(stderr, "lstat '%s' failed: %s\n", path, strerror(e));
		if(e != ENOENT) {
			printf("??????????   ? ?      ?      ?        \?\?\?\?-\?\?-\?\? \?\?:\?\? %s\n", name);
		}
		return -1;
	}

	mode2str(s.st_mode, mode);
#ifdef _WIN32
	sprintf(user, "%u", s.st_uid);
	sprintf(group, "%u", s.st_gid);
#else
	user2str(s.st_uid, user);
	group2str(s.st_gid, group);
#endif

	strftime(date, 32, "%Y-%m-%d %H:%M", localtime((const time_t *)&s.st_mtime));
	date[31] = 0;

	// 12345678901234567890123456789012345678901234567890123456789012345678901234567890
	// MMMMMMMMMM LLL UUUUUU GGGGGG XXXXXXXX YYYY-MM-DD HH:MM NAME (->SLINKTARGET)

	switch(s.st_mode & S_IFMT) {
		case S_IFDIR:
			COLOR_PRINT("1;34", file, name);
			printf("%s %3u %-6s %-6s          %s %s\n",	
				mode, (unsigned int)s.st_nlink, user, group, date, file);
			break;
#ifndef _WIN32
		case S_IFBLK:
		case S_IFCHR:
			COLOR_PRINT("1;33", file, name);
			printf("%s %3u %-6s %-6s %3d, %3d %s %s\n",
				mode, (unsigned int)s.st_nlink,
				user, group, (int)MAJOR(s.st_rdev),
				(int)MINOR(s.st_rdev), date, file);
			break;
#endif
		case S_IFREG:
			//#if defined _WIN32 && !defined _WIN32_WCE
#ifdef _WIN32
			// Some versions of Windows can't handle the type long long int properly in printf
			printf("%s %3u %-6s %-6s %8ld %s %s\n", mode, (unsigned int)s.st_nlink, user, group, s.st_size, date, name);
#else
			if(access(path, X_OK) < 0) {
				COLOR_PRINT("0;37", file, name);
				printf("%s %3u %-6s %-6s %8lld %s %s\n",
					mode, (unsigned int)s.st_nlink, user,
					group, (long long int)s.st_size, date, file);
			} else {
				COLOR_PRINT("1;32", file, name);
				printf("%s %3u %-6s %-6s %8lld %s %s\n",
					mode, (unsigned int)s.st_nlink, user,
					group, (long long int)s.st_size, date, file);
			}
#endif
			break;
#if !defined _WIN32 || defined _WIN32_WNT_NATIVE
		case S_IFLNK: {
			COLOR_PRINT("1;36", file, name);
			char linkto[256];
			int len;

			len = readlink(path, linkto, 256);
			if(len < 0) return -1;

			if(len > 255) {
				linkto[252] = '.';
				linkto[253] = '.';
				linkto[254] = '.';
				linkto[255] = 0;
			} else {
				linkto[len] = 0;
			}

			printf("%s %3u %-6s %-6s          %s %s -> %s\n",
				mode, (unsigned int)s.st_nlink, user, group, date, file, linkto);
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

	if((flags & LIST_DIRECTORIES) || !(name = strrchr(path, '/'))) name = path;
	else name++;

	if(lstat(path, &s) < 0) {
		return -1;
	}

	lgetfilecon(path, &maclabel);
	if(!maclabel) {
		//return -1;
		//maclabel = "?";
		if(!(flags & LIST_LONG)) return -1;
	}

	mode2str(s.st_mode, mode);
	user2str(s.st_uid, user);
	group2str(s.st_gid, group);

	switch(s.st_mode & S_IFMT) {
		case S_IFLNK: {
			char linkto[256];
			ssize_t len;

			len = readlink(path, linkto, sizeof(linkto));
			if(len < 0) return -1;

			if((size_t)len > sizeof(linkto)-1) {
				linkto[sizeof(linkto)-4] = '.';
				linkto[sizeof(linkto)-3] = '.';
				linkto[sizeof(linkto)-2] = '.';
				linkto[sizeof(linkto)-1] = 0;
			} else {
				linkto[len] = 0;
			}

			printf("%s %-8s %-8s          %s %s -> %s\n",
				mode, user, group, maclabel, name, linkto);
			break;
		}
		default:
			printf("%s %-8s %-8s          %s %s\n",
				mode, user, group, maclabel, name);

	}

	free(maclabel);

	return 0;
}
#endif

static int listfile(const char *dirname, const char *filename, int flags) {
	struct stat s;
	char file[4096];
	char tmp[4096];
	const char *name;
	const char* pathname = filename;

	if (dirname != NULL) {
		const char *slash = "/";
		if(dirname[strlen(dirname) - 1] == '/') slash = "";
		snprintf(tmp, sizeof(tmp), "%s%s%s", dirname, slash, filename);
		pathname = tmp;
	} else {
		pathname = filename;
	}

	if ((flags & (LIST_LONG | LIST_SIZE | LIST_CLASSIFY)) == 0) {
		/* There almost same with listfile_long() function, It should be replace use define or new function. */
		/* name is anything after the final '/', or the whole path if none*/
		if((flags & LIST_DIRECTORIES) || !(name = strrchr(pathname, '/'))) name = pathname;
		else name++;

		if(lstat(pathname, &s) < 0) {
			return -1;
		}
		switch(s.st_mode & S_IFMT) {
			case S_IFDIR:
				COLOR_PRINT("1;34", file, name);
				puts(file);
				break;
			case S_IFBLK:
			case S_IFCHR:
				COLOR_PRINT("1;33", file, name);
				puts(file);
				break;
			case S_IFREG:
				if(access(pathname, X_OK) < 0) {
					COLOR_PRINT("0;37", file, name);
					puts(file);
				} else {
					COLOR_PRINT("1;32", file, name);
					puts(file);
				}
				break;
#ifndef _WIN32
			case S_IFLNK:
				COLOR_PRINT("1;36", file, name);
				puts(file);
				break;
#endif
			default:
				puts(file);
		}
		return 0;
	}

#ifndef _NO_SELINUX
	if(flags & LIST_MACLABEL) {
		return listfile_maclabel(pathname, flags);
	} else
#endif
	if ((flags & LIST_LONG) != 0) {
		return listfile_long(pathname, flags);
	} else /*((flags & LIST_SIZE) != 0)*/ {
		return listfile_size(pathname, filename, flags);
	}
}

static int listdir(const char *name, int flags)
{
	char tmp[4096];
	DIR *d;
	struct dirent *de;
	strlist_t files = STRLIST_INITIALIZER;
	int list_all = flags & LIST_ALL;

	d = opendir(name);
	if(d == 0) {
		fprintf(stderr, "opendir failed, %s\n", strerror(errno));
		return -1;
	}

	if (flags & LIST_SIZE || flags & LIST_LONG) {
		show_total_size(name, d, flags);
	}

	while((de = readdir(d))) {
		//printf("de = %p\n", de);
		if(!list_all && (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0)) continue;
		if(de->d_name[0] == '.' && !list_all && !(flags & LIST_ALL_ALMOST)) continue;

		//fprintf(stderr, "toolbox debug: de->d_name = %p\"%s\"\n", de->d_name, de->d_name);
		strlist_append_dup(&files, de->d_name);
	}

	strlist_sort(&files);
	STRLIST_FOREACH(&files, filename, listfile(name, filename, flags));
	strlist_done(&files);

	if (flags & LIST_RECURSIVE) {
		strlist_t subdirs = STRLIST_INITIALIZER;

		rewinddir(d);

		while ((de = readdir(d)) != 0) {
			struct stat s;
			int err;

			if(!list_all && (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0)) continue;
			if(de->d_name[0] == '.' && !list_all && !(flags & LIST_ALL_ALMOST)) continue;

			if(strcmp(name, "/") == 0) {
				snprintf(tmp, sizeof(tmp), "/%s", de->d_name);
			} else {
				snprintf(tmp, sizeof(tmp), "%s/%s", name, de->d_name);
			}

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

static int listpath(const char *name, int flags)
{
	struct stat s;
	int err;

	/*
	 * If the name ends in a '/', use stat() so we treat it like a
	 * directory even if it's a symlink.
	 */
	err = (name[strlen(name)-1] == '/' ? stat : lstat)(name, &s);

	if (err < 0) {
		perror(name);
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

int ls_main(int argc, char **argv)
{
	int flags = 0;

	if(argc > 1) {
		int i;
		int err = 0;
		int end_of_options = 0;
		strlist_t  files = STRLIST_INITIALIZER;

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
							flags &= ~LIST_ALL_ALMOST;
							break;
						case 'F': flags |= LIST_CLASSIFY; break;
						case '-':
							if(!arg[1]) end_of_options = 1;
							else {
								const char *long_arg = arg + 1;
								if(strcmp(long_arg, "color") == 0) {
									// Doing
									is_color = true;
								} else if(strncmp(long_arg, "color=", 6) == 0) {
									// TODO
								} else if(strcmp(long_arg, "help") == 0) {
									fprintf(stderr, "Usage: %s [-lsRdAaF] [--color]"
#ifdef _WIN32_WCE
										" <file>"
#endif
										" [<file>] ...\n", argv[0]);
									return 0;
								} else {
									fprintf(stderr, "%s: Unknown option '%s'. Aborting.\n", argv[0], argv[i]);
									return 1;
								}
							}
							goto out_of_inner_loop;
						default:
							fprintf(stderr, "%s: Unknown option '-%c'. Aborting.\n", argv[0], arg[0]);
							exit(1);
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

		if(files.count > 0) {
			if(files.count > 1) flags |= MULTI_FILES;
			STRLIST_FOREACH(&files, path, {
				if (listpath(path, flags) != 0) {
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
	return listpath(".", flags);
#endif
}
