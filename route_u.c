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
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#if defined __linux__
#include <linux/route.h>
#elif defined __MACH__	/* For the GNU/Hurd and Mac OS X, but they are both failed */
#ifdef __GNU__		/* The GNU/Hurd system seems not implements some ioctls, porting failed */
#include <hurd/ioctl.h>
#ifndef SIOCADDRT
#define SIOCADDRT _IOW('r', 10, struct ortentry)
#endif
#endif
#include <net/route.h>
#endif

static inline int set_address(const char *address, struct sockaddr *sa) {
    return inet_aton(address, &((struct sockaddr_in *)sa)->sin_addr);
}

/* current support the following routing entries */
/* route add default dev wlan0 */
/* route add default gw 192.168.1.1 dev wlan0 */
/* route add -net 192.168.1.2 netmask 255.255.255.0 gw 192.168.1.1 */

int route_main(int argc, char *argv[])
{
    struct rtentry rt = {
        .rt_dst     = {.sa_family = AF_INET},
        .rt_genmask = {.sa_family = AF_INET},
        .rt_gateway = {.sa_family = AF_INET},
    };

    errno = EINVAL;
    if (argc > 2 && strcmp(argv[1], "add") == 0) {
        if (strcmp(argv[2], "default") == 0) {
            /* route add default dev wlan0 */
            if (argc > 4 && !strcmp(argv[3], "dev")) {
                rt.rt_flags = RTF_UP;
                rt.rt_dev = argv[4];
                errno = 0;
                goto apply;
            }

            /* route add default gw 192.168.1.1 dev wlan0 */
            if (argc > 6 && strcmp(argv[3], "gw") == 0 && strcmp(argv[5], "dev") == 0) {
                rt.rt_flags = RTF_UP | RTF_GATEWAY;
                rt.rt_dev = argv[6];
                if (set_address(argv[4], &rt.rt_gateway)) {
                    errno = 0;
                }
                goto apply;
            }
        }

        /* route add -net 192.168.1.2 netmask 255.255.255.0 gw 192.168.1.1 */
        if(argc > 7 && (strcmp(argv[2], "-net") == 0 || strcmp(argv[2], "--net") == 0) &&
        strcmp(argv[4], "netmask") == 0) {
            if(strcmp(argv[6], "gw") == 0) {
                rt.rt_flags = RTF_UP | RTF_GATEWAY;
                if(set_address(argv[3], &rt.rt_dst) &&
                set_address(argv[5], &rt.rt_genmask) &&
                set_address(argv[7], &rt.rt_gateway)) {
                    errno = 0;
                }
                goto apply;
            } else if(strcmp(argv[6], "dev") == 0) {
                rt.rt_flags = RTF_UP;
                rt.rt_dev = argv[7];
                if (set_address(argv[3], &rt.rt_dst) &&
                set_address(argv[5], &rt.rt_genmask)) {
                    errno = 0;
                }
                goto apply;
            }
        }
    }

apply:
    if (!errno) {
        int s = socket(AF_INET, SOCK_DGRAM, 0);
        if (s != -1 && (ioctl(s, SIOCADDRT, &rt) != -1 || errno == EEXIST)) {
            return 0;
        }
    }
    puts(strerror(errno));
    return errno;
}
