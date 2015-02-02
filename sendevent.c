#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/input.h>
#include <errno.h>
#include <getopt.h>

int main(int argc, char *argv[]) {
    //int i;
    int fd;
    int ret;
    int version;
    struct input_event event;

    if(argc != 5) {
        fprintf(stderr, "Usage: %s <device> <type> <code> <value>\n", argv[0]);
        return -1;
    }

    fd = open(argv[1], O_RDWR);
    if(fd == -1) {
        fprintf(stderr, "could not open %s, %s\n", argv[optind], strerror(errno));
        return 1;
    }
    if(ioctl(fd, EVIOCGVERSION, &version)) {
        fprintf(stderr, "could not get driver version for %s, %s\n", argv[optind], strerror(errno));
        return 1;
    }
    memset(&event, 0, sizeof(event));
    event.type = atoi(argv[2]);
    event.code = atoi(argv[3]);
    event.value = atoi(argv[4]);
    ret = write(fd, &event, sizeof(event));
    if(ret < sizeof(event)) {
        fprintf(stderr, "write event failed, %s\n", strerror(errno));
        return -1;
    }
    return 0;
}
