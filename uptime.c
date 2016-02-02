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

#include <sys/time.h>
#if defined _WIN32 && !defined _WIN32_WNT_NATIVE
#include "timefunc.h"
//#define CLOCK_MONOTONIC 1
//#include <windows.h>
#elif defined __linux__
//#include <sys/ioctl.h>
//#include <linux/rtc.h>
#elif defined __APPLE__ || defined __FreeBSD__
#include <sys/sysctl.h>
#endif
#if !defined _WIN32 && !defined _NO_UTMPX
#include <utmpx.h>
#endif
//#include <linux/android_alarm.h>
#include <fcntl.h>
#include <stdio.h>
#include <time.h>
#include <math.h>


#ifdef __sun
#undef NAN
#define NAN (__builtin_nanf(""))
#if isnan == __builtin_isnan
#include <ieeefp.h>
#undef isnan
#define isnan isnand
#endif
#elif !defined NAN
#define NAN (__builtin_nanf(""))
#endif

//#ifdef __APPLE__
//#define _USE_SYSCTL
//#endif

static void format_time(int time, char *buffer) {
	int seconds, minutes, hours, days;

	seconds = time % 60;
	time /= 60;
	minutes = time % 60;
	time /= 60;
	hours = time % 24;
	days = time / 24;

	if(days > 0) {
		sprintf(buffer, "%d days, %02d:%02d:%02d", days, hours, minutes, seconds);
	} else {
		sprintf(buffer, "%02d:%02d:%02d", hours, minutes, seconds);
	}
}
/*
int64_t elapsedRealtime()
{
    struct timespec ts;
    int fd, result;

    fd = open("/dev/alarm", O_RDONLY);
    if (fd < 0)
        return fd;

   result = ioctl(fd, ANDROID_ALARM_GET_TIME(ANDROID_ALARM_ELAPSED_REALTIME), &ts);
   close(fd);

    if (result == 0)
        return ts.tv_sec;
    return -1;
}*/

int main() {
	float run_time = NAN;
	char total_string[100];
	//float elapsed;
#if defined __APPLE__ || defined __FreeBSD__
	struct timeval boot_tv;
	size_t len = sizeof boot_tv;
	int mib[2] = { CTL_KERN, KERN_BOOTTIME };
	if(sysctl(mib, 2, &boot_tv, &len, NULL, 0) == 0) {
		run_time = difftime(time(NULL), boot_tv.tv_sec);
	}
#elif defined __linux__ || defined _WIN32
	struct timespec up_timespec;
#ifdef __linux__
	float total_time = NAN, idle_time = NAN;
	char idle_string[100], sleep_string[100];
	int run_time_only = 0;
	FILE *file = fopen("/proc/uptime", "r");
	if(file) {
		if(fscanf(file, "%f %f", &total_time, &idle_time) < 2) {
			fprintf(stderr, "Could not parse /proc/uptime\n");
			fclose(file);
			return 1;
		}
		fclose(file);
	}
/*
	char up[9];
	int fd = open("/proc/uptime", O_RDONLY);
	if(fd == -1) {
		perror("/proc/uptime");
		return 1;
	}
	if(read(fd, up, 8) < 0) {
		perror("read: /proc/uptime");
		return 1;
	}
	if(close(fd) < 0) perror("close");
	up[8] = 0;
	total_time = atof(up);
*/
#endif
	if(clock_gettime(CLOCK_MONOTONIC, &up_timespec) < 0) {
		fprintf(stderr, "Could not get monotonic time\n");
		return 2;
	}
	run_time = up_timespec.tv_sec + up_timespec.tv_nsec / 1e9;
#endif
/*
	elapsed = elapsedRealtime();
	if (elapsed < 0) {
		fprintf(stderr, "elapsedRealtime failed\n");
		return -1;
	}
*/
#ifndef _WIN32
#ifdef __linux__
	if(isnan(total_time)) {
#else
	if(isnan(run_time)) {
#endif
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
#ifdef __linux__
			run_time_only = 1;
#else
			fprintf(stderr, "Could not get up time\n");
			return 2;
#endif
#ifndef _NO_UTMPX
		}
#ifdef __linux__
		total_time = up;
#else
		run_time = up;
#endif
#endif
	}
#endif

#ifdef __linux__
	if(!run_time_only) {
		format_time(total_time, total_string);
		format_time((int)idle_time, idle_string);
		format_time((int)(total_time - run_time), sleep_string);
		printf("up time: %s;  idle time: %s;  sleep time: %s\n", total_string, idle_string, sleep_string);
		return 0;
	}
#endif
	format_time(run_time, total_string);
	printf("up time: %s\n", total_string);
	return 0;
}
