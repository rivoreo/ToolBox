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
#ifdef SIOCSIFHWADDR
#include <net/if_arp.h>
#endif

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

#ifdef SIOCSIFHWADDR
static unsigned char atoxb(const char *s) {
	unsigned char r = *s - (isalpha(*s) ? (islower(*s) ? 87 : 55) : '0');
	if(*++s) r = (*s - (isalpha(*s) ? (islower(*s) ? 87 : 55) : '0')) + r * 16;
	return r;
}

static int etheraddr_to_sockaddr(struct sockaddr *sa, const char *ether) {
	sa->sa_family = ARPHRD_ETHER;
	unsigned char *p = (unsigned char *)sa->sa_data;
	int i;
	for(i=0; i<6; i++) {
		if(*ether == ':' || *ether == '-') ether++;
		if(!isxdigit(*ether) || !isxdigit(ether[1])) {
			return -1;
		}
		p[i] = atoxb(ether);
		ether += 2;
	}
	//return *ether ? -1 : 0;
	if(*ether && *ether != ':' && *ether != '-') return -1;
	return 0;
}

static void sethwaddr(int s, struct ifreq *ifr, const char *hwaddr) {
	if(ioctl(s, SIOCGIFHWADDR, ifr)) die("SIOCGIFHWADDR");
	if(ifr->ifr_hwaddr.sa_family != ARPHRD_ETHER) {
		fprintf(stderr, "error: setting hardware address for %s: only ethernet interfaces are supported\n", ifr->ifr_name);
		exit(1);
	}
	if(etheraddr_to_sockaddr(&ifr->ifr_hwaddr, hwaddr) < 0) {
		fprintf(stderr, "error: invalid ethernet address '%s'\n", hwaddr);
		exit(1);
	}
	if(ioctl(s, SIOCSIFHWADDR, ifr)) die("SIOCSIFHWADDR");
}
#endif

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

static int getmetric(int s, struct ifreq *ifr) {
	// Both metric=0 or ioctl errors cases metric not get displayed
	return ioctl(s, SIOCGIFMETRIC, ifr) < 0 ? 0 : ifr->ifr_metric;
}

static void setmetric(int s, struct ifreq *ifr, const char *metric) {
	int m = atoi(metric);
	ifr->ifr_metric = m;
	if(ioctl(s, SIOCSIFMETRIC, ifr) < 0) die("SIOCSIFMETRIC");
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
static void setnetmask(int s, struct ifreq *ifr, const struct sockaddr_in *addr) {
#if defined __sun && defined __SVR4
	memcpy(&ifr->ifr_addr, addr, sizeof(struct sockaddr));
#else
	memcpy(&ifr->ifr_netmask, addr, sizeof(struct sockaddr));
#endif
	if(ioctl(s, SIOCSIFNETMASK, ifr) < 0) die("SIOCSIFNETMASK");
}

static void setnetmask_s(int s, struct ifreq *ifr, const char *addr) {
	struct sockaddr_in sa;
	init_sockaddr_in(&sa, addr);
	setnetmask(s, ifr, &sa);
}

static void setprefixlen(int s, struct ifreq *ifr, int prefixlen) {
	//fprintf(stderr, "function: setprefixlen(%d, %p, %d)\n", s, ifr, prefixlen);
	struct sockaddr_in netmask = {
		.sin_family = AF_INET,
		.sin_addr.s_addr = htonl(0xffffffff << (32 - prefixlen))
	};
	setnetmask(s, ifr, &netmask);
}
#endif

static void setaddr(int s, struct ifreq *ifr, const char *addr) {
	int prefixlen = -1;
	char *addr_dup = strdup(addr);
	if(!addr_dup) die("strdup");
	char *slash = strrchr(addr_dup, '/');
	if(slash) {
		char *endptr;
		if(strchr(addr_dup, '/') != slash) {
			fprintf(stderr, "error: invalid address '%s'\n", addr);
			exit(1);
		}
		prefixlen = strtoul(slash + 1, &endptr, 10);
		if(*endptr) {
			fprintf(stderr, "error: invalid prefix length '%s'\n", slash + 1);
			exit(1);
		}
#ifdef BSD
		fprintf(stderr, "warning: setting prefix length is currently not supported\n");
//#else
//		setprefixlen(s, ifr, prefixlen);
#endif
		*slash = 0;
	}
	init_sockaddr_in((struct sockaddr_in *)&ifr->ifr_addr, addr_dup);
	free(addr_dup);
	if(ioctl(s, SIOCSIFADDR, ifr) < 0) die("SIOCSIFADDR");
#ifndef BSD
	if(prefixlen >= 0) setprefixlen(s, ifr, prefixlen);
#endif
}

static int print_status(int s, struct ifreq *ifr) {
	unsigned int addr, mask, flags, mtu, metric;
	char astring[20];
	char mstring[20];
	const char *updown, *brdcst, *loopbk, *ppp, *running, *multi, *noarp;
	int have_address = 0;

	//fprintf(stderr, "function: print_status(%d, %p)\n", s, ifr);

	if(ioctl(s, SIOCGIFADDR, ifr) == 0) {
		have_address = 1;
		addr = ((struct sockaddr_in *)&ifr->ifr_addr)->sin_addr.s_addr;
		if (ioctl(s, SIOCGIFNETMASK, ifr) < 0) return -1;
		mask = ((struct sockaddr_in *)&ifr->ifr_addr)->sin_addr.s_addr;

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
	}

	if (ioctl(s, SIOCGIFFLAGS, ifr) < 0) return -1;
	flags = ifr->ifr_flags;

	metric = getmetric(s, ifr);
	mtu = getmtu(s, ifr);

	printf("%s: ", ifr->ifr_name);
	if(have_address) printf("ip %s mask %s ", astring, mstring);

	updown =  (flags & IFF_UP)           ? "up" : "down";
	brdcst =  (flags & IFF_BROADCAST)    ? " broadcast" : "";
	loopbk =  (flags & IFF_LOOPBACK)     ? " loopback" : "";
	ppp =     (flags & IFF_POINTOPOINT)  ? " point-to-point" : "";
	running = (flags & IFF_RUNNING)      ? " running" : "";
	multi =   (flags & IFF_MULTICAST)    ? " multicast" : "";
	noarp =   (flags & IFF_NOARP)        ? " noarp" : "";
	printf("flags [%s%s%s%s%s%s%s]", updown, brdcst, loopbk, ppp, running, multi, noarp);
	if(metric) printf(" metric %u", metric);
	if(mtu) printf(" mtu %u", mtu);
	putchar('\n');

	return 0;
}

static int print_status_all(int fd) {
	//char buffer[sizeof(struct ifreq)];
	struct ifconf ifc;
	int n;
#ifdef SIOCGIFNUM
	if(ioctl(fd, SIOCGIFNUM, &n) < 0) {
		perror("error: SIOCGIFNUM");
		return 1;
	}
	ifc.ifc_len = sizeof(struct ifreq) * n;
#else
	// FIXME: This desn't work on BSD
	ifc.ifc_len = 0;
	ifc.ifc_buf = NULL;
	if(ioctl(fd, SIOCGIFCONF, &ifc) < 0) {
		perror("error: SIOCGIFCONF");
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
		perror("error: SIOCGIFCONF");
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

static void print_usage(const char *name, int show_options) {
	fprintf(stderr, "ifconfig - toolbox " VERSION "\n"
		"Copyright 2007-2015 PC GO Ld.\n"
		"Copyright 2015-2016 Rivoreo\n\n"
		"Usage:\n"
		"	%s -a\n"
		"	%s <interface> [<address>[/<prefix-len>]]%s\n\n",
		name, name,
		show_options ? "\n"
			"		[netmask <netmask>]\n"
			"		[broadcast <broad-addr>]\n"
			"		[{destination|pointopoint} <dest-addr>]\n"
			"		[metric <metric>]\n"
			"		[mtu <mtu>]\n"
#ifdef SIOCSIFHWADDR
			"		[{ether|hw} <hw-addr>]\n"
#endif
			"		[up|down]" : " [<options>]");
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
					print_usage(argv[0], 1);
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
		print_usage(argv[0], 0);
		fprintf(stderr, "%s: use '-h' for options\n", argv[0]);
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
#ifdef SIOCSIFHWADDR
		} else if(strcmp(argv[0], "ether") == 0 || strcmp(argv[0], "hw") == 0) {
			if(argc < 2) {
				fprintf(stderr, "error: expecting an hardware address for parameter \"%s\"\n", argv[0]);
				return 1;
			}
			argc--, argv++;
			sethwaddr(s, &ifr, argv[0]);
#endif
		} else if(strcmp(argv[0], "mtu") == 0) {
			argc--, argv++;
			if (!argc) {
				errno = EINVAL;
				die("expecting a value for parameter \"mtu\"");
			}
			setmtu(s, &ifr, argv[0]);
		} else if(strcmp(argv[0], "metric") == 0) {
			argc--, argv++;
			if (!argc) {
				errno = EINVAL;
				die("expecting a value for parameter \"metric\"");
			}
			setmetric(s, &ifr, argv[0]);
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
		} else if(strcmp(argv[0], "netmask") == 0) {
			argc--, argv++;
			if (!argc) { 
				errno = EINVAL;
				die("expecting an IP address for parameter \"netmask\"");
			}
#ifdef BSD
			fprintf(stderr, "error: setting netmask is currently not supported\n");
			return 1;
#else
			setnetmask_s(s, &ifr, argv[0]);
#endif
		} else {
			setaddr(s, &ifr, argv[0]);
			setflags(s, &ifr, IFF_UP, 0);
		}
		argc--, argv++;
	}
	return 0;
}
