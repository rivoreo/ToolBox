/*
 * 工具箱中的某工具
 * 版权所有 2007-2015 PC GO Ld.
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

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sched.h>

static void usage(const char *s)
{
	fprintf(stderr, "Usage: %s [[-r] <priority pids ...>] [-g <pid>]\n", s);
	exit(EXIT_FAILURE);
}

void print_prio(pid_t pid)
{
	int sched;
	struct sched_param sp;

	printf("pid %d's priority: %d\n", pid, getpriority(PRIO_PROCESS, pid));

	printf("scheduling class: ");
	sched = sched_getscheduler(pid);
	switch(sched) {
		case SCHED_FIFO:
			puts("FIFO");
			break;
		case SCHED_RR:
			puts("RR");
			break;
		case SCHED_OTHER:
			puts("Normal");
			break;
		case -1:
			perror("sched_getscheduler");
			break;
		default:
			puts("Unknown");
	}

	sched_getparam(pid, &sp);
	printf("RT prio: %d (of %d to %d)\n", sp.sched_priority,
			sched_get_priority_min(sched), sched_get_priority_max(sched));
}

int main(int argc, char *argv[])
{
	int prio;
	int realtime = 0;
	char *name = argv[0];

	// consume command name
	argc--;
	argv++;

	if(argc < 1) usage(name);

	if(strcmp("-r", argv[0]) == 0) {
		// do realtime priority adjustment
		realtime = 1;
		argc--;
		argv++;
	}

	if(strcmp("-g", argv[0]) == 0) {
		if(argc < 2) usage(name);
		print_prio(atoi(argv[1]));
		return 0;
	}

	if(argc < 1) usage(name);

	prio = atoi(argv[0]);
	argc--;
	argv++;

	if(argc < 1) usage(name);

	while(argc) {
		pid_t pid;

		pid = atoi(argv[0]);
		argc--;
		argv++;

		if(realtime) {
			struct sched_param sp = { .sched_priority = prio };
			int ret;

			ret = sched_setscheduler(pid, SCHED_RR, &sp);
			if(ret) {
				perror("sched_set_scheduler");
				exit(EXIT_FAILURE);
			}
		} else {
			int ret;

			ret = setpriority(PRIO_PROCESS, pid, prio);
			if(ret) {
				perror("setpriority");
				exit(EXIT_FAILURE);
			}
		}
	}

	return 0;
}
