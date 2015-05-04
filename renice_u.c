/*
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

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sched.h>
#include <ctype.h>

static void usage(const char *s) {
#if defined __APPLE__ || defined __INTERIX
	fprintf(stderr, "Usage: %s <priority> <pid> [...]\n", s);
#else
	fprintf(stderr, "Usage: %s { [-r] <priority> <pid> [...] | -g <pid> }\n", s);
#endif
}

#if !defined __APPLE__ && !defined __INTERIX
void print_prio(pid_t pid) {
	int sched;
	struct sched_param sp;

	printf("pid %d's priority: %d\n", pid, getpriority(PRIO_PROCESS, pid));

	printf("scheduling class: ");
	sched = sched_getscheduler(pid);
	switch(sched) {
		case -1:
			perror("sched_getscheduler");
			break;
		case SCHED_FIFO:
			puts("FIFO");
			break;
		case SCHED_RR:
			puts("RR");
			break;
		case SCHED_OTHER:
			puts("Normal");
			break;
		default:
			puts("Unknown");
	}

	if(sched_getparam(pid, &sp) < 0) {
		perror("sched_getparam");
		return;
	}
	printf("RT prio: %d (of %d to %d)\n", sp.sched_priority,
		sched_get_priority_min(sched), sched_get_priority_max(sched));
}
#endif

int renice_main(int argc, char *argv[]) {
	int prio;
	int r = 0;
#if !defined __APPLE__ && !defined __INTERIX
	int realtime = 0;
	while(1) {
		int c = getopt(argc, argv, "rg:h");
		if(c == -1) break;
		switch(c) {
			case 'r':
				realtime = 1;
				break;
			case 'g':
				if(!isdigit(*optarg)) {
					fprintf(stderr, "%s: Invalid pid '%s'\n", argv[0], optarg);
					return -2;
				}
				print_prio(atoi(optarg));
				return 0;
			case 'h':
				usage(argv[0]);
				return 0;
			case '?':
				return -1;
		}
	}
#endif
	//fprintf(stderr, "argc = %d, optind = %d\n", argc, optind);
	if(argc < optind + 2) {
		usage(argv[0]);
		return -1;
	}

	prio = atoi(argv[optind++]);
	//argc -= optind;
	argv += optind;

	while(*argv) {
		pid_t pid = atoi(*argv++);
#if !defined __APPLE__ && !defined __INTERIX
		if(realtime) {
			struct sched_param sp = { .sched_priority = prio };
			if(sched_setscheduler(pid, SCHED_RR, &sp) < 0) {
				perror("sched_set_scheduler");
				//return EXIT_FAILURE;
				r++;
			}
		} else {
#endif
			if(setpriority(PRIO_PROCESS, pid, prio) < 0) {
				perror("setpriority");
				//return EXIT_FAILURE;
				r++;
			}
#if !defined __APPLE__ && !defined __INTERIX
		}
#endif
	}

	return r;
}
