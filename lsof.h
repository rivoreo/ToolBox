#ifndef _LSOF_H
#define _LSOF_H

#include <sys/types.h>
#include <limits.h>

#define BUF_MAX 1024
#define COMMAND_DISPLAY_MAX (9 + 1)
#define USER_DISPLAY_MAX (10 + 1)

typedef struct {
	pid_t pid;
	char user[USER_DISPLAY_MAX];
	char command[COMMAND_DISPLAY_MAX];
	char path[PATH_MAX];
	ssize_t parent_length;
} pid_info_t;

#endif
