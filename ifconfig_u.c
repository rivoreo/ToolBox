/*	ifconfig - toolbox
	Copyright 2007-2015 PC GO Ld.
	Copyright 2015-2016 Rivoreo

	This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#include "version.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include <errno.h>
#include <string.h>
#include <ctype.h>

#include <sys/socket.h>
#include <netinet/in.h>
#ifdef __linux__
#include <linux/if.h>
#include <linux/sockios.h>
#else
#include <net/if.h>
#include <sys/param.h>
#ifdef __sun
#include <sys/sockio.h>
#endif
#endif
#include <arpa/inet.h>

static void die(const char *s) {
	fprintf(stderr,"error: %s (%s)\n", s, strerror(errno));
	exit(1);
}

#ifdef __SVR4
static struct lifreq *get_lifreq(const struct ifreq *ifr) {
	static struct lifreq lifr;
	//memset(&lifr, 0, sizeof lifr);
	size_t name_len = strlen(ifr->ifr_name) + 1;
	if(name_len > sizeof lifr.lifr_name) return NULL;
	memcpy(lifr.lifr_name, ifr->ifr_name, name_len);
	return &lifr;
}
#endif

static void setflags(int s, struct ifreq *ifr, int set, int clr) {
	if(ioctl(s, SIOCGIFFLAGS, ifr) < 0) die("SIOCGIFFLAGS");
	ifr->ifr_flags = (ifr->ifr_flags & (~clr)) | set;
	if(ioctl(s, SIOCSIFFLAGS, ifr) < 0) die("SIOCSIFFLAGS");
}

static inline void init_sockaddr_in(struct sockaddr_in *sin, const char *addr) {
	sin->sin_family = AF_INET;
	sin->sin_port = 0;
	sin->sin_addr.s_addr = inet_addr(addr);
}

static unsigned int getmtu(int s, struct ifreq *ifr) {
#ifdef __SVR4
	struct lifreq *lifr = get_lifreq(ifr);
	if(!lifr) {
		//fprintf(stderr, "error: name too long\n");
		//exit(1);
		return 0;
	}
	return ioctl(s, SIOCGLIFMTU, lifr) < 0 ? 0 : lifr->lifr_mtu;
#else
	return ioctl(s, SIOCGIFMTU, ifr) < 0 ? 0 : ifr->ifr_mtu;
#endif
}

static void setmtu(int s, struct ifreq *ifr, const char *mtu) {
	int m = atoi(mtu);
#ifdef __SVR4
	struct lifreq *lifr = get_lifreq(ifr);
	if(!lifr) {
		fprintf(stderr, "error: name too long\n");
		exit(1);
	}
	lifr->lifr_mtu = m;
	if(ioctl(s, SIOCSLIFMTU, lifr) < 0) die("SIOCSIFMTU");
#else
	ifr->ifr_mtu = m;
	if(ioctl(s, SIOCSIFMTU, ifr) < 0) die("SIOCSIFMTU");
#endif
}

static void setdstaddr(int s, struct ifreq *ifr, const char *addr) {
	init_sockaddr_in((struct sockaddr_in *)&ifr->ifr_dstaddr, addr);
	if(ioctl(s, SIOCSIFDSTADDR, ifr) < 0) die("SIOCSIFDSTADDR");
}

#if !defined BSD
static void setnetmask(int s, struct ifreq *ifr, const char *addr) {
#if defined __sun && defined __SVR4
	init_sockaddr_in((struct sockaddr_in *)&ifr->ifr_addr, addr);
#else
	init_sockaddr_in((struct sockaddr_in *)&ifr->ifr_netmask, addr);
#endif
	if(ioctl(s, SIOCSIFNETMASK, ifr) < 0) die("SIOCSIFNETMASK");
}
#endif

static void setaddr(int s, struct ifreq *ifr, const char *addr) {
	init_sockaddr_in((struct sockaddr_in *)&ifr->ifr_addr, addr);
	if(ioctl(s, SIOCSIFADDR, ifr) < 0) die("SIOCSIFADDR");
}

static int print_status(int s, struct ifreq *ifr) {
	unsigned int addr, mask, flags, mtu;
	char astring[20];
	char mstring[20];
	const char *updown, *brdcst, *loopbk, *ppp, *running, *multi;

	//fprintf(stderr, "function: print_status(%d, %p)\n", s, ifr);

	if (ioctl(s, SIOCGIFADDR, ifr) < 0) return -1;
	addr = ((struct sockaddr_in *)&ifr->ifr_addr)->sin_addr.s_addr;

	if (ioctl(s, SIOCGIFNETMASK, ifr) < 0) return -1;
	mask = ((struct sockaddr_in *)&ifr->ifr_addr)->sin_addr.s_addr;

	if (ioctl(s, SIOCGIFFLAGS, ifr) < 0) return -1;
	flags = ifr->ifr_flags;

	mtu = getmtu(s, ifr);

	sprintf(astring, "%d.%d.%d.%d",
		addr & 0xff,
		((addr >> 8) & 0xff),
		((addr >> 16) & 0xff),
		((addr >> 24) & 0xff));
	sprintf(mstring, "%d.%d.%d.%d",
		mask & 0xff,
		((mask >> 8) & 0xff),
		((mask >> 16) & 0xff),
		((mask >> 24) & 0xff));
	printf("%s: ip %s mask %s flags [", ifr->ifr_name, astring, mstring);

	updown =  (flags & IFF_UP)           ? "up" : "down";
	brdcst =  (flags & IFF_BROADCAST)    ? " broadcast" : "";
	loopbk =  (flags & IFF_LOOPBACK)     ? " loopback" : "";
	ppp =     (flags & IFF_POINTOPOINT)  ? " point-to-point" : "";
	running = (flags & IFF_RUNNING)      ? " running" : "";
	multi =   (flags & IFF_MULTICAST)    ? " multicast" : "";
	printf("%s%s%s%s%s%s]", updown, brdcst, loopbk, ppp, running, multi);
	if(mtu) printf(" mtu %u\n", mtu);

	return 0;
}

int ifconfig_main(int argc, char *argv[]) {
	struct ifreq ifr;
	int s;

	argc--;
	argv++;

	if(argc == 0) {
		puts("ifconfig - toolbox " VERSION "\n"
			"Copyright 2007-2015 PC GO Ld.\n"
			"Copyright 2015-2016 Rivoreo\n\n"
			"Usage: ifconfig <interface> [<options>]");
		return 0;
	}

	memset(&ifr, 0, sizeof(struct ifreq));
	strncpy(ifr.ifr_name, argv[0], IFNAMSIZ);
	ifr.ifr_name[IFNAMSIZ-1] = 0;
	argc--, argv++;

	if((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		die("cannot open control socket\n");
	}

	if (argc == 0) {
		if(print_status(s, &ifr) < 0) {
			perror(ifr.ifr_name);
			return 1;
		}
		return 0;
	}

	while(argc > 0) {
		if(strcmp(argv[0], "up") == 0) {
			setflags(s, &ifr, IFF_UP, 0);
		} else if(strcmp(argv[0], "mtu") == 0) {
			argc--, argv++;
			if (!argc) {
				errno = EINVAL;
				die("expecting a value for parameter \"mtu\"");
			}
			setmtu(s, &ifr, argv[0]);
		} else if(strcmp(argv[0], "-pointopoint") == 0) {
			setflags(s, &ifr, IFF_POINTOPOINT, 1);
		} else if(strcmp(argv[0], "pointopoint") == 0) {
			argc--, argv++;
			if(!argc) {
				errno = EINVAL;
				die("expecting an IP address for parameter \"pointtopoint\"");
			}
			setdstaddr(s, &ifr, argv[0]);
			setflags(s, &ifr, IFF_POINTOPOINT, 0);
		} else if(strcmp(argv[0], "down") == 0) {
			setflags(s, &ifr, 0, IFF_UP);
#if !defined BSD
		} else if(strcmp(argv[0], "netmask") == 0) {
			argc--, argv++;
			if (!argc) { 
				errno = EINVAL;
				die("expecting an IP address for parameter \"netmask\"");
			}
			setnetmask(s, &ifr, argv[0]);
#endif
		} else if(isdigit(argv[0][0])) {
			setaddr(s, &ifr, argv[0]);
			setflags(s, &ifr, IFF_UP, 0);
		}
		argc--, argv++;
	}
	return 0;
}
