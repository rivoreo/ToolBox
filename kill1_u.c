/*	kill1 - toolbox
	Copyright 2007-2015 PC GO Ld.
	Copyright 2015 libdll.so

	This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#include <sys/types.h>
#include <sys/ptrace.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <ctype.h>

static const char *signals[] = {
	"NONE", "HUP", "INT", "QUIT", "ILL", "TRAP", "ABRT", "BUS", "FPE", "KILL", "USR1", "SEGV",
	"USR2", "PIPE", "ALRM", "TERM", "STKFLT", "CHLD", "CONT", "STOP", "TSTP", "TTIN", "TTOU",
	"URG", "XCPU", "XFSZ", "VTALRM", "PROF", "WINCH", "IO", "PWR", "SYS"
};

static int get_signal_from_name(const char *signal_name) {
	if(!signal_name || !*signal_name) return -1;
	int i;
	for(i=1; i<32; i++) if(strcmp(signal_name, signals[i]) == 0) return i;
	if(!isdigit(*signal_name)) return -1;
	return atoi(signal_name);
}

static void print_signals() {
	int i;
	for(i=1; i<32; i++) {
		printf("%2d %s%s", i, signals[i], (i % 4 && i != 31) ? (strlen(signals[i]) > 4 ? "	" : "		") : "\n");
		//putchar(i % 4 ? '	' : '\n');
	}
}

static void print_usage() {
	fprintf(stderr, "Usage: kill1 [<options>]\n\n"
			"options:\n"
			"	-s <signal>	Specify the <signal> to send\n"
			"	-n		Keep going when ptrace failed\n"
			"	-l		List all available signals\n"
			"	-v		Verbose\n\n");
}

int kill1_main(int argc, char **argv) {
	int no_stop_flag = 0;
	int verbose = 0;
	char *signal1 = "TERM";
first_loop:
	while(*++argv) {
		if(strcmp(*argv, "--help") == 0) {
			print_usage();
			return -1;
		} else if(*argv[0] == '-') {
			const char *arg = *argv;
			if(!arg[1]) goto eoption;
			while(*++arg) switch(*arg) {
				case 'h':
					print_usage();
					return -1;
				case 'l':
					print_signals();
					return 0;
				case 'n':
					no_stop_flag = 1;
					continue;
				case 's':
					if(arg[1]) {
						fprintf(stderr, "Option '-s' not the end of a set of combined options\n");
						return -2;
					}
					signal1 = *++argv;
					if(!signal1) {
						fprintf(stderr, "Option '-s' need an argument\n");
						return -2;
					}
					goto first_loop;
				case 'v':
					verbose = 1;
					continue;

				eoption:
				default:
					fprintf(stderr, "Unknown option '%s'\n", *argv);
					return -2;
			}
		}
	}

	int signal2 = get_signal_from_name(signal1);
	if(signal2 < 0) {
		fprintf(stderr, "Unknown signal '%s'\n", signal1);
		return -3;
	}

	int r = 0;
	if(verbose) fprintf(stderr, "Attaching to process 1...	");
	if(ptrace(PT_ATTACH, 1, NULL, 0) < 0) {
		//perror("ptrace");
		int e = errno;
		if(e) {
			fprintf(stderr, "ptrace: %s\n", strerror(e));
			if(no_stop_flag) {
				r = 1;
				goto send_signal;
			}
			return 1;
		}
	}

	waitpid(1, NULL, 0);
	if(verbose) fprintf(stderr, "OK\n");

send_signal:
	if(verbose) fprintf(stderr, "Sending signal %s (%d) ...	", signal1, signal2);
	//int r = 0;
	if(kill(1, signal2) < 0) {
		perror("kill");
		//return 2;
		//r += 2;
		r |= 1 << 1;
	} else if(verbose) fprintf(stderr, "OK\n");

	if(r & 1) return r;

	if(verbose) fprintf(stderr, "Detaching from process 1...	");
	if(ptrace(PT_DETACH, 1, NULL, 0) < 0) {
		int e = errno;
		if(e) {
			if(e != ESRCH || verbose) fprintf(stderr, "ptrace: %s\n", strerror(e));
			//r += 3;
			r |= 1 << 2;
		}
	}
	if(verbose && !(r & (1 << 2))) fprintf(stderr, "OK\n");

	return r;
}
