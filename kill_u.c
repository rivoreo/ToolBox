#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>

static struct {
    unsigned int number;
    char *name;
} signals[] = {
#define _SIG(name) {SIG##name, #name}
    /* Single Unix Specification signals */
    _SIG(ABRT),
    _SIG(ALRM),
    _SIG(FPE),
    _SIG(HUP),
    _SIG(ILL),
    _SIG(INT),
    _SIG(KILL),
    _SIG(PIPE),
    _SIG(QUIT),
    _SIG(SEGV),
    _SIG(TERM),
    _SIG(USR1),
    _SIG(USR2),
    _SIG(CHLD),
    _SIG(CONT),
    _SIG(STOP),
    _SIG(TSTP),
    _SIG(TTIN),
    _SIG(TTOU),
    _SIG(BUS),
#ifdef SIGPOLL
    _SIG(POLL),
#endif
    _SIG(PROF),
    _SIG(SYS),
    _SIG(TRAP),
    _SIG(URG),
    _SIG(VTALRM),
    _SIG(XCPU),
    _SIG(XFSZ),
    /* non-SUS signals */
    _SIG(IO),
#ifdef SIGPWR
    _SIG(PWR),
#endif
#ifdef SIGSTKFLT
    _SIG(STKFLT),
#endif
    _SIG(WINCH),
#undef _SIG
};

/* To indicate a matching signal was not found */
#define SENTINEL (-1)

void list_signals()
{
    unsigned int sorted_signals[NSIG];
    unsigned int i;
    unsigned int num;

    memset(sorted_signals, SENTINEL, sizeof(sorted_signals));

    // Sort the signals
    for(i = 0; i < sizeof(signals)/sizeof(signals[0]); i++) {
        sorted_signals[signals[i].number] = i;
    }

    num = 0;
    for(i = 1; i < NSIG; i++) {
        unsigned int index = sorted_signals[i];
        if(index == SENTINEL) {
            continue;
        }

        fprintf(stderr, "%2d) SIG%-9s ", i, signals[index].name);

        if((num++ % 4) == 3) {
            fprintf(stderr, "\n");
        }
    }

    if(num % 4) {
        fprintf(stderr, "\n");
    }
}

unsigned int name_to_signal(const char* name)
{
    unsigned int i;

    for (i = 1; i < sizeof(signals) / sizeof(signals[0]); i++) {
        if (!strcasecmp(name, signals[i].name)) {
            return signals[i].number;
        }
    }

    return SENTINEL;
}

int kill_main(int argc, char **argv)
{
    unsigned int sig = SIGTERM;
    int result = 0;

    argc--;
    argv++;

    if (argc >= 1 && argv[0][0] == '-') {
        size_t arg_len = strlen(argv[0]);
        if (arg_len < 2) {
            fprintf(stderr, "invalid argument: -\n");
            return -1;
        }

		char *arg = *argv + 1;
		if(arg_len == 2) {
			switch(*arg) {
				case 'l':
					list_signals();
					return 0;
				case 'h':
					goto usage;
				case 's':
					sig = name_to_signal(argv[2]);
					if(sig == SENTINEL) {
						sig = atoi(argv[2]);
						if(!sig) {
							fprintf(stderr, "Invalid signal '%s'\n", argv[2]);
							return 1;
						}
					}
			}
		}

        argc--;
        argv++;
    }

	if(!argc) {
usage:
		fprintf(stderr, "Usage: kill { [-s <signal>] <pid> | -l }\n");
		return -1;
	}

    while(argc > 0){
        int pid = atoi(argv[0]);
        int err = kill(pid, sig);
        if (err < 0) {
            result = err;
            fprintf(stderr, "could not kill pid %d: %s\n", pid, strerror(errno));
        }

        argc--;
        argv++;
    }

    return result;
}
