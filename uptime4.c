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
#ifdef _WIN32
#include "timefunc.h"
#define CLOCK_MONOTONIC 1
//#include <windows.h>
#else
#include <linux/ioctl.h>
#include <linux/rtc.h>
#endif
//#include <linux/android_alarm.h>
#include <fcntl.h>
#include <stdio.h>
#include <time.h>


static void format_time(int time, char *buffer) {
	int seconds, minutes, hours, days;

	seconds = time % 60;
	time /= 60;
	minutes = time % 60;
	time /= 60;
	hours = time % 24;
	days = time / 24;

	if (days > 0)
		sprintf(buffer, "%d days, %02d:%02d:%02d", days, hours, minutes, seconds);
	else
		sprintf(buffer, "%02d:%02d:%02d", hours, minutes, seconds);
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

int main(int argc, char *argv[])
{
	float total_time, run_time, idle_time;
	char total_string[100], idle_string[100], sleep_string[100];
	//float elapsed;
	struct timespec up_timespec;

#ifndef _WIN32
	FILE *file = fopen("/proc/uptime", "r");
	if (!file) {
		fprintf(stderr, "Could not open /proc/uptime\n");
		return -1;
	}
	if (fscanf(file, "%*f %f", &idle_time) != 1) {
		fprintf(stderr, "Could not parse /proc/uptime\n");
		fclose(file);
		return -1;
	}
	fclose(file);
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
	total_time = atoi(up);
#endif
	if (clock_gettime(CLOCK_MONOTONIC, &up_timespec) < 0) {
		fprintf(stderr, "Could not get monotonic time\n");
		return 2;
	}
	run_time = up_timespec.tv_sec + up_timespec.tv_nsec / 1e9;
/*
	elapsed = elapsedRealtime();
	if (elapsed < 0) {
		fprintf(stderr, "elapsedRealtime failed\n");
		return -1;
	}
*/
#ifdef _WIN32
	format_time(run_time, total_string);
	char tick_string[100];
	format_time(GetTickCount() / 1000, tick_string);
	printf("up time: %s, tick count: %s\n", total_string, tick_string);
#else
	format_time(total_time, total_string);
	format_time((int)idle_time, idle_string);
	format_time((int)(total_time - run_time), sleep_string);
	printf("up time: %s;  idle time: %s;  sleep time: %s\n", total_string, idle_string, sleep_string);
#endif
	return 0;
}
