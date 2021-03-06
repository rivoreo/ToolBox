/*
 * 工具箱中的某工具
 * 版权所有 2007-2015 PC GO Ld.
 * 版权所有 2015-2016 Rivoreo
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "lsof.h"
#include <pwd.h>
#include <sys/stat.h>

#include <assert.h>

static void print_header() {
	printf("%-16s %5s %10s %4s %9s %12s %10s %8s %s\n",
	"COMMAND", "PID", "USER", "FD", "TYPE", "DEVICE", "SIZE/OFF", "NODE", "NAME");
}

static void print_type(const char *type, pid_info_t *info) {
	static ssize_t link_dest_size;
	static char link_dest[PATH_MAX];

	//strncat(info->path, type, sizeof info->path);
	//info->path[sizeof info->path - 1] = 0;
	size_t type_len = strlen(type);
	if(info->parent_length + type_len > sizeof info->path) {
		assert(info->parent_length > sizeof info->path);
		type_len = sizeof info->path - info->parent_length;
	}
	memcpy(info->path + info->parent_length, type, type_len);
	info->path[info->parent_length + type_len] = 0;
	if((link_dest_size = readlink(info->path, link_dest, sizeof(link_dest) - 1)) < 0) {
		if(errno == ENOENT) goto out;
		snprintf(link_dest, sizeof(link_dest), "%s (readlink: %s)", info->path, strerror(errno));
	} else {
		link_dest[link_dest_size] = '\0';
	}

	// Things that are just the root filesystem are uninteresting (we already know)
	if(strcmp(link_dest, "/") == 0) goto out;

	printf("%-16s %5d %10s %4s %9s %12s %9s %9s %s\n",
		info->command, (int)info->pid, info->user, type,
		"???", "???", "???", "???", link_dest);

out:
	info->path[info->parent_length] = 0;
}

// Prints out all file that have been memory mapped
static void print_maps(pid_info_t *info) {
	FILE *maps;
	//char buffer[PATH_MAX + 100];

	size_t offset;
	//int major, minor;
	char device[10];
	long int inode;
	char file[PATH_MAX];

	strncat(info->path, "maps", sizeof info->path);

	maps = fopen(info->path, "r");
	if(!maps) goto out;

	while(fscanf(maps, "%*x-%*x %*s %zx %5s %ld %s\n", &offset, device, &inode,
				file) == 4) {
		// We don't care about non-file maps
		if(inode == 0 || strcmp(device, "00:00") == 0) continue;

		printf("%-16s %5d %10s %4s %9s %12s %9zd %9ld %s\n",
			info->command, (int)info->pid, info->user, "mem",
			"???", device, offset, inode, file);
	}

	fclose(maps);

out:
	info->path[info->parent_length] = 0;
}

// Prints out all open file descriptors
static void print_fds(pid_info_t *info) {
	static char *fd_path = "fd/";
	strncat(info->path, fd_path, sizeof(info->path));

	int previous_length = info->parent_length;
	info->parent_length += strlen(fd_path);

	DIR *dir = opendir(info->path);
	if(dir == NULL) {
		char msg[BUF_MAX];
		snprintf(msg, sizeof(msg), "%s (opendir: %s)", info->path, strerror(errno));
		printf("%-16s %5d %10s %4s %9s %12s %9s %9s %s\n",
			info->command, (int)info->pid, info->user, "FDS",
			"", "", "", "", msg);
		goto out;
	}

	struct dirent *de;
	while((de = readdir(dir))) {
		if(strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0) continue;
		print_type(de->d_name, info);
	}
	closedir(dir);

out:
	info->parent_length = previous_length;
	info->path[info->parent_length] = 0;
}

void lsof_dumpinfo(pid_t pid) {
	int fd;
	pid_info_t info;
	struct stat pidstat;
	struct passwd *pw;

	info.pid = pid;
	snprintf(info.path, sizeof(info.path), "/proc/%d/", (int)pid);
	info.parent_length = strlen(info.path);

	// Get the UID by calling stat on the proc/pid directory.
	if(stat(info.path, &pidstat) == 0) {
		pw = getpwuid(pidstat.st_uid);
		if(pw) {
			strcpy(info.user, pw->pw_name);
		} else {
			snprintf(info.user, USER_DISPLAY_MAX, "%d", (int)pidstat.st_uid);
		}
	} else {
		strcpy(info.user, "???");
	}

#ifdef __linux__
	// Read the command line information; each argument is terminated with NULL.
	strncat(info.path, "cmdline", sizeof info.path);
	fd = open(info.path, O_RDONLY);
	if(fd == -1) {
		fprintf(stderr, "Couldn't read %s\n", info.path);
		return;
	}

	char command[PATH_MAX];
	int numRead = read(fd, command, sizeof(command) - 1);
	close(fd);

	if(numRead < 0) {
		fprintf(stderr, "Error reading command: %s: %s\n", info.path, strerror(errno));
		return;
	}

	command[numRead] = 0;

	// We only want the basename
	strncpy(info.command, basename(command), COMMAND_DISPLAY_MAX - 1);
	info.command[COMMAND_DISPLAY_MAX - 1] = 0;
#else
	strncat(info.path, "psinfo", sizeof info.path);
	fd = open(info.path, O_RDONLY);
	if(fd == -1) {
		fprintf(stderr, "Couldn't read %s\n", info.path);
		return;
	}
	//if(lseek(fd, 0x68, SEEK_SET) == (off_t)-1) {
	if(lseek(fd, 0x58, SEEK_SET) == (off_t)-1) {
		fprintf(stderr, "Couldn't find command in %s, %s\n", info.path, strerror(errno));
		return;
	}
	char command[PATH_MAX];
	int numRead = read(fd, command, sizeof(command) - 1);
	close(fd);
	if(numRead < 0) {
		fprintf(stderr, "Error reading command: %s: %s\n", info.path, strerror(errno));
		return;
	}
	strncpy(info.command, command, COMMAND_DISPLAY_MAX - 1);
#endif

	// Read each of these symlinks
	print_type("cwd", &info);
	print_type("exe", &info);
	print_type("root", &info);

	print_fds(&info);
	print_maps(&info);
}

int main(int argc, char *argv[]) {
	long int pid = 0;
	char *endptr;
	if(argc == 2) {
		pid = strtol(argv[1], &endptr, 10);
	}

	print_header();

	if(pid) {
		lsof_dumpinfo(pid);
	} else {
		DIR *dir = opendir("/proc");
		if(dir == NULL) {
			fprintf(stderr, "Couldn't open /proc\n");
			return -1;
		}

		struct dirent *de;
		while((de = readdir(dir))) {
			if(strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0) continue;

			// Only inspect directories that are PID numbers
			pid = strtol(de->d_name, &endptr, 10);
			if(*endptr) continue;

			lsof_dumpinfo(pid);
		}
		closedir(dir);
	}

	return 0;
}
