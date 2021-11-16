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

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/param.h>
#include <unistd.h>
#include <string.h>
#if defined __GNU__ && defined __MACH__
#include <ddekit/timer.h>
#endif

struct state {
	long int procs_r;
	long int procs_b;

	long int mem_free;
	long int mem_mapped;
	long int mem_anon;
	long int mem_slab;

	long int sys_in;
	long int sys_cs;
	long int sys_flt;

	long int cpu_us;
	long int cpu_ni;
	long int cpu_sy;
	long int cpu_id;
	long int cpu_wa;
	long int cpu_ir;
	long int cpu_si;
};

#define MAX_LINE 256

char line[MAX_LINE];

static void read_state(struct state *s);
static int read_meminfo(struct state *s);
static int read_stat(struct state *s);
static int read_vmstat(struct state *s);
static void print_header(void);
static void print_line(struct state *old, struct state *new);
static void usage(char *command);

int main(int argc, char *argv[]) {
	struct state s[2];
	int toggle, count;
	int i;

	int iterations = 1;
	int delay = 1;
	int header_interval = 20;
	for (i = 1; i < argc; i++) {
		if(strcmp(argv[i], "-n") == 0) { 
			if(i++ >= argc - 1) {
				fprintf(stderr, "Option -n requires an argument.\n");
				//exit(EXIT_FAILURE);
				return EXIT_FAILURE;
			}
			iterations = strcasecmp(argv[i], "inf") == 0 ? -1 : atoi(argv[i]);
			continue;
		}
		if(strcmp(argv[i], "-d") == 0) {
			if(i >= argc - 1) {
				fprintf(stderr, "Option -d requires an argument.\n");
				//exit(EXIT_FAILURE);
				return EXIT_FAILURE;
			}
			delay = atoi(argv[++i]);
			continue;
		}
		if(strcmp(argv[i], "-r") == 0) {
			if(i >= argc - 1) {
				fprintf(stderr, "Option -r requires an argument.\n");
				//exit(EXIT_FAILURE);
				return EXIT_FAILURE;
			}
			header_interval = atoi(argv[++i]);
			continue;
		}
		if(strcmp(argv[i], "-h") == 0) {
			usage(argv[0]);
			//exit(EXIT_SUCCESS);
			return EXIT_SUCCESS;
		}
		fprintf(stderr, "Invalid argument \"%s\".\n", argv[i]);
		usage(argv[0]);
		//exit(EXIT_FAILURE);
		return EXIT_FAILURE;
	}

	toggle = 0;
	count = 0;

	read_state(s + 1);
	while ((iterations < 0) || (iterations-- > 0)) {
		sleep(delay);
		if (header_interval) {
			if (count == 0) print_header();
			count = (count + 1) % header_interval;
		}
		read_state(s + toggle);
		print_line(s + (1 - toggle), s + toggle);
		toggle = 1 - toggle;
	}

	return 0;
}

static void read_state(struct state *s) {
	int error;

	error = read_meminfo(s);
	if (error) {
		fprintf(stderr, "vmstat: could not read /proc/meminfo: %s\n", strerror(error));
		exit(EXIT_FAILURE);
	}

	error = read_stat(s);
	if (error) {
		fprintf(stderr, "vmstat: could not read /proc/stat: %s\n", strerror(error));
		exit(EXIT_FAILURE);
	}

	error = read_vmstat(s);
	if (error) {
		fprintf(stderr, "vmstat: could not read /proc/vmstat: %s\n", strerror(error));
		exit(EXIT_FAILURE);
	}
}

static int read_meminfo(struct state *s) {
	FILE *f;

	f = fopen("/proc/meminfo", "r");
	if (!f) return errno;

	while (fgets(line, MAX_LINE, f)) {
		sscanf(line, "MemFree: %ld kB", &s->mem_free);
		sscanf(line, "AnonPages: %ld kB", &s->mem_anon);
		sscanf(line, "Mapped: %ld kB", &s->mem_mapped);
		sscanf(line, "Slab: %ld kB", &s->mem_slab);
	}

	fclose(f);

	return 0;
}

static int read_stat(struct state *s) {
	FILE *f;

	f = fopen("/proc/stat", "r");
	if (!f) return errno;

	while (fgets(line, MAX_LINE, f)) {
		if (!strncmp(line, "cpu ", 4)) {
			sscanf(line, "cpu  %ld %ld %ld %ld %ld %ld %ld",
					&s->cpu_us, &s->cpu_ni, &s->cpu_sy, &s->cpu_id, &s->cpu_wa,
					&s->cpu_ir, &s->cpu_si);
		}
		sscanf(line, "intr %ld", &s->sys_in);
		sscanf(line, "ctxt %ld", &s->sys_cs);
		sscanf(line, "procs_running %ld", &s->procs_r);
		sscanf(line, "procs_blocked %ld", &s->procs_b);
	}

	fclose(f);

	return 0;
}

static int read_vmstat(struct state *s) {
	FILE *f;

	f = fopen("/proc/vmstat", "r");
	if (!f) return errno;

	while (fgets(line, MAX_LINE, f)) {
		sscanf(line, "pgmajfault %ld", &s->sys_flt);
	}

	fclose(f);

	return 0;
}

static void print_header(void) {
	printf("%-5s  %-27s  %-14s  %-17s\n", "procs", "memory", "system", "cpu");
	printf("%2s %2s  %6s %6s %6s %6s  %4s %4s %4s  %2s %2s %2s %2s %2s %2s\n", "r", "b", "free", "mapped", "anon", "slab", "in", "cs", "flt", "us", "ni", "sy", "id", "wa", "ir");
}

/* Jiffies to percent conversion */
#define JP(jif) ((jif) * 100 / (HZ))
#define NORM(var) ((var) = (((var) > 99) ? (99) : (var)))

static void print_line(struct state *old, struct state *new) {
	int us, ni, sy, id, wa, ir;
	us = JP(new->cpu_us - old->cpu_us); NORM(us);
	ni = JP(new->cpu_ni - old->cpu_ni); NORM(ni);
	sy = JP(new->cpu_sy - old->cpu_sy); NORM(sy);
	id = JP(new->cpu_id - old->cpu_id); NORM(id);
	wa = JP(new->cpu_wa - old->cpu_wa); NORM(wa);
	ir = JP(new->cpu_ir - old->cpu_ir); NORM(ir);
	printf("%2ld %2ld  %6ld %6ld %6ld %6ld  %4ld %4ld %4ld  %2d %2d %2d %2d %2d %2d\n",
		new->procs_r ? (new->procs_r - 1) : 0, new->procs_b,
		new->mem_free, new->mem_mapped, new->mem_anon, new->mem_slab,
		new->sys_in - old->sys_in, new->sys_cs - old->sys_cs, new->sys_flt - old->sys_flt,
		us, ni, sy, id, wa, ir);
}

static void usage(char *command) {
	fprintf(stderr, "Usage: %s [ -h ] [ -n <iterations> ] [ -d <delay> ] [ -r <header_repeat> ]\n"
			"    -n <iterations>    How many rows of data to print.\n"
			"    -d <delay>         How long to sleep between rows.\n"
			"    -r <header_repeat> How many rows to print before repeating the header. Zero means never repeat.\n"
			"    -h                 Displays this help screen.\n",
		command);
}
