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

#include <sys/time.h>
#if defined __APPLE__ || defined __FreeBSD__
#include <sys/sysctl.h>
#else
#include <string.h>
#endif
#if !defined _WIN32 && !defined _NO_UTMPX
#include <utmpx.h>
#endif
#include <fcntl.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#ifndef NAN
#define NAN (__builtin_nanf(""))
#endif

#ifdef __sun
#if NAN == __builtin_nan
#undef NAN
#define NAN (nan(NULL))
#endif
#if isnan == __builtin_isnan
#include <ieeefp.h>
#undef isnan
#define isnan isnand
#endif
#endif

static void format_time(int time, char* buffer) {
    int seconds, minutes, hours, days;

    seconds = time % 60;
    time /= 60;
    minutes = time % 60;
    time /= 60;
    hours = time % 24;
    days = time / 24;

    if(days > 0) sprintf(buffer, "%d days, %02d:%02d:%02d", days, hours, minutes, seconds);
    else sprintf(buffer, "%02d:%02d:%02d", hours, minutes, seconds);
}

int uptime_main() {
	float up_time = NAN;
	char up_string[100];
#if defined __APPLE__ || defined __FreeBSD__
	struct timeval boot_tv;
	size_t len = sizeof boot_tv;
	int mib[2] = { CTL_KERN, KERN_BOOTTIME };
	if(sysctl(mib, 2, &boot_tv, &len, NULL, 0) == 0) {
		up_time = difftime(time(NULL), boot_tv.tv_sec);
	}
#elif defined __linux__ || defined _WIN32
	struct timespec up_timespec;
#ifdef __linux__
	float idle_time;
	char idle_string[100];
    FILE* file = fopen("/proc/uptime", "r");
    if(!file) {
        fprintf(stderr, "Could not open /proc/uptime\n");
	strcpy(idle_string, "unknown");
    } else {
		if(fscanf(file, "%*f %f", &idle_time) != 1) {
			fprintf(stderr, "Could not parse /proc/uptime\n");
			//fclose(file);
			//return -1;
			strcpy(idle_string, "unknown");
		} else format_time(idle_time, idle_string);
		fclose(file);
    }
#endif
    if(clock_gettime(CLOCK_MONOTONIC, &up_timespec) == 0) {
        up_time = up_timespec.tv_sec + up_timespec.tv_nsec / 1e9;
    }
#endif

#ifndef _WIN32
	if(isnan(up_time)) {
#ifndef _NO_UTMPX
		int up = -1;
		setutxent();
		struct utmpx *t;
		while((t = getutxent())) {
			if(t->ut_type == BOOT_TIME) {
				up = time(NULL) - t->ut_tv.tv_sec;
				break;
			}
		}
		if(up < 0) {
#endif
			fprintf(stderr, "Could not get up time\n");
			return 2;
#ifndef _NO_UTMPX
		}
		up_time = up;
#endif
	}
#endif

	format_time(up_time, up_string);
#ifdef __linux__
	printf("up time: %s,  idle time: %s\n", up_string, idle_string);
#else
	printf("up time: %s\n", up_string);
#endif
	return 0;
}
