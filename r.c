#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <string.h>

static void usage() {
	fprintf(stderr,"r [-b|-s] <address> [<value>]\n");
	//return -1;
}

int main(int argc, char *argv[]) {
	int width = 4, set = 0, fd;
	unsigned int addr, value, endaddr = 0;
	unsigned long mmap_start, mmap_size;
	void *page;
	char *end;

	if(argc < 2 || (argv[1][0] == '-' && argc < 3)) {
		usage();
		return -1;
	}

	if(strcmp(argv[1], "-b") == 0) {
		width = 1;
		argc--;
		argv++;
	} else if(strcmp(argv[1], "-s") == 0) {
		width = 2;
		argc--;
		argv++;
	} else if(strcmp(argv[1], "-h") == 0) {
		usage();
		return 0;
	}

	//if(argc < 2) return usage();

	addr = strtoul(argv[1], 0, 16);

	end = strchr(argv[1], '-');
	if(end) endaddr = strtoul(end + 1, 0, 16);

	if(!endaddr) endaddr = addr + width - 1;

	if(endaddr <= addr) {
		fprintf(stderr, "invalid end address\n");
		return -1;
	}

	if(argc > 2) {
		set = 1;
		value = strtoul(argv[2], 0, 16);
	}

	fd = open("/dev/mem", O_RDWR | O_SYNC);
	if(fd == -1) {
		fprintf(stderr,"cannot open /dev/mem\n");
		return -1;
	}

	mmap_start = addr ;
	mmap_size = endaddr - mmap_start + 1;

	page = mmap(0, mmap_size, PROT_READ | PROT_WRITE,
		MAP_SHARED, fd, mmap_start);

	if(page == MAP_FAILED) {
		fprintf(stderr,"cannot mmap region\n");
		return -1;
	}

	while(addr <= endaddr) {
		switch(width) {
			case 4: {
				unsigned *x = (unsigned*)(((char *)page) + (addr & 4095));
				if(set) *x = value;
				fprintf(stderr,"%08x: %08x\n", addr, *x);
				break;
			}
			case 2: {
				unsigned short *x = (unsigned short*)(((char *)page) + (addr & 4095));
				if(set) *x = value;
				fprintf(stderr,"%08x: %04x\n", addr, *x);
				break;
			}
			case 1: {
				unsigned char *x = (unsigned char*)(((char *)page) + (addr & 4095));
				if(set) *x = value;
				fprintf(stderr,"%08x: %02x\n", addr, *x);
				break;
			}
		}
		addr += width;
	}
	return 0;
}
