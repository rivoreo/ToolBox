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
#include <netdb.h>

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
	if(isdigit(*addr)) {
		sin->sin_addr.s_addr = inet_addr(addr);
	} else {
		struct addrinfo hints = {
			.ai_family = PF_INET,
			.ai_socktype = 0,
			.ai_protocol = 0
		};
		struct addrinfo *info;
		int e = getaddrinfo(addr, NULL, &hints, &info);
		if(e) {
			fprintf(stderr, "error: cannot resolve host '%s': %s\n", addr, gai_strerror(e));
			exit(1);
		}
		sin->sin_addr = ((struct sockaddr_in *)info->ai_addr)->sin_addr;
		freeaddrinfo(info);
	}
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

static void setbroadaddr(int s, struct ifreq *ifr, const char *addr) {
	init_sockaddr_in((struct sockaddr_in *)&ifr->ifr_broadaddr, addr);
	if(ioctl(s, SIOCSIFBRDADDR, ifr) < 0) die("SIOCSIFBRDADDR");
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

static int print_status_all(int fd) {
	//char buffer[sizeof(struct ifreq)];
	struct ifconf ifc;
	int n;
#ifdef SIOCGIFNUM
	if(ioctl(fd, SIOCGIFNUM, &n) < 0) {
		perror("SIOCGIFNUM");
		return 1;
	}
	ifc.ifc_len = sizeof(struct ifreq) * n;
#else
	// FIXME: This desn't work on BSD
	ifc.ifc_len = 0;
	ifc.ifc_buf = NULL;
	if(ioctl(fd, SIOCGIFCONF, &ifc) < 0) {
		perror("SIOCGIFCONF");
		return 1;
		//die("SIOCGIFCOUNT");
	}
	n = ifc.ifc_len / sizeof(struct ifreq);
#endif
	ifc.ifc_buf = malloc(ifc.ifc_len);
	if(!ifc.ifc_len) {
		perror("malloc");
		return 1;
	}

	//fprintf(stderr, "n = %d, ifc_len = %d, ifc_buf = %p\n", n, ifc.ifc_len, ifc.ifc_buf);

	if(ioctl(fd, SIOCGIFCONF, &ifc) < 0) {
		perror("SIOCGIFCONF");
		return 1;
	}

	int i;
	for(i=0; i<n; i++) {
		if(print_status(fd, ifc.ifc_req + i) < 0) {
			perror(ifc.ifc_req[i].ifr_name);
		}
	}
	return 0;
}

static void print_usage(const char *name) {
	fprintf(stderr, "ifconfig - toolbox " VERSION "\n"
		"Copyright 2007-2015 PC GO Ld.\n"
		"Copyright 2015-2016 Rivoreo\n\n"
		"Usage:\n"
		"	%s -a\n"
		"	%s <interface> [<address>[/<prefix-len>]] [<options>]\n\n",
		name, name);
}

int ifconfig_main(int argc, char *argv[]) {
	struct ifreq ifr;
	int s;

	int all = 0;
	char **v = argv + 1;

	while(*v) {
		if(**v == '-') {
			char *o = *v + 1;
			switch(*o) {
				case 'a':
					all = 1;
					break;
				case 'h':
					print_usage(argv[0]);
					return 0;
				default:
					fprintf(stderr, "%s: Unknown option '-%c'\n", argv[0], *o);
					return -1;
			}
			argv[1] = argv[0];
			argc--;
			argv++;
		} else break;
		v++;
	}

	if((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		die("cannot open control socket\n");
	}

	if(all) {
		//print_status_all(s);
		//return 0;
		return print_status_all(s);
	}

	if(argc < 2) {
		close(s);
		print_usage(argv[0]);
		return -1;
	}

	argc--;
	argv++;

	memset(&ifr, 0, sizeof(struct ifreq));
	strncpy(ifr.ifr_name, argv[0], IFNAMSIZ);
	ifr.ifr_name[IFNAMSIZ-1] = 0;
	argc--, argv++;

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
		} else if(strcmp(argv[0], "broadcast") == 0) {
			if(argc < 2) {
				fprintf(stderr, "error: expecting an IP address for parameter \"%s\"\n", argv[0]);
				return 1;
			}
			argc--, argv++;
			setbroadaddr(s, &ifr, argv[0]);
			setflags(s, &ifr, IFF_BROADCAST, 0);
		} else if(strcmp(argv[0], "destination") == 0 || strcmp(argv[0], "pointopoint") == 0) {
			if(argc < 2) {
				//errno = EINVAL;
				fprintf(stderr, "error: expecting an IP address for parameter \"%s\"\n", argv[0]);
				return 1;
			}
			argc--, argv++;
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
		} else {
			setaddr(s, &ifr, argv[0]);
			setflags(s, &ifr, IFF_UP, 0);
		}
		argc--, argv++;
	}
	return 0;
}
