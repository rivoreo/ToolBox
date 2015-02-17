/*	A getopt implementation with GNU extensions
	Copyright 2007-2015 PC GO Ld.

	This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

/* Basd on "mingwrt-3.15.2-mingw32/mingwex/getopt.c" (1.8, keithmarshall) */

#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <stdlib.h>

#define GETOPT_LONG_NO_MATCH 0
#define GETOPT_LONG_ABBREVIATED_MATCH 1
#define GETOPT_LONG_EXACT_MATCH 2

int optind = 1;
int opterr = 1;
int optopt = '?';
char *optarg;

static int getopt_missing_arg(const char *optstring) {
	if(*optstring == '+' || *optstring == '-') optstring++;
	return *optstring == ':' ? ':' : '?';
}

static void getopt_argerror(const char *format, const char *program, const struct option *opt) {
	if(opterr) {
		fprintf(stderr, "%s: ", program);
		//fprintf(stderr, fmt, opt ? "--" : "-", 
		fprintf(stderr, format, "--", opt->name);
	}
	optopt = opt->val;
}


#define GETOPT_SET_CONVENTIONS 0x1000
#define GETOPT_POSIXLY_CORRECT 0x0010

static int getopt_conventions(int flags) {
	static int conventions = 0;
	if(conventions == 0 && !(flags & GETOPT_SET_CONVENTIONS)) {
		conventions = GETOPT_SET_CONVENTIONS;
		if(flags == '+' || getenv("POSIXLY_CORRECT")) {
			conventions |= GETOPT_POSIXLY_CORRECT;
		}
	} else if(flags & GETOPT_SET_CONVENTIONS) {
		conventions |= flags;
	}
	return conventions;
}

static const char *getopt_match(char lookup, const char *optstring) {
	if(*optstring == '+' || *optstring == '-') optstring++;
	if(*optstring == ':') optstring++;
	do {
		if(*optstring == lookup) return optstring;
	} while(*++optstring);
	return NULL;
}

static int getopt_match_long(char *nextchar, const char *optname) {
	char matchc;
	while((matchc = *nextchar++) && matchc == *optname) optname++;
	if(matchc) {
		if(matchc != '=') return GETOPT_LONG_NO_MATCH;
		optarg = nextchar;
	}
	return *optname ? GETOPT_LONG_ABBREVIATED_MATCH : GETOPT_LONG_EXACT_MATCH;
}

static int getopt_resolved(int argc, char *const *argv, int *argind, const struct option *opt, int index, int *rindex, const char *optstring) {
	if(rindex) *rindex = index;
	optind = *argind + 1;
	if(optarg && opt[index].has_arg == no_argument) {
		getopt_argerror("option `%s%s' doesn't accept an argument\n", argv[0], opt + index);
		return '?';
	} else if(!optarg && opt[index].has_arg == required_argument) {
		if(optind < argc) {
			optarg = argv[*argind = optind++];
		} else {
			getopt_argerror("option `%s%s' requires an argument\n", argv[0], opt + index);
			return getopt_missing_arg(optstring);
		}
	}
	if(opt[index].flag) {
		*(opt[index].flag) = opt[index].val;
		return 0;
	}
	return opt[index].val;
}


static int getopt_parse(int argc, char *const *argv, const char *optstring, const struct option *long_opts, int *index, int long_only) {
	static int argind = 0;
	static char *nextchar = NULL;
	static int optbase = 0;
	static int optmark = 0;

	if(optind < optbase) {
		// Reinitialize required
		if(!optind) optind = 1;
		optmark = optbase = argind = optind - 1;
		nextchar = NULL;
	}

	if(argc < 1) return -1;

	if(nextchar && *nextchar) {
		const char *optchar = getopt_match(optopt = *nextchar++, optstring);
		if(optchar) {
			if(optchar[1] == ':') {
				optarg = nextchar;
				if(!*nextchar) {
					if(optchar[2] == ':') optarg = NULL;
					else if(argc - argind > 1) optarg = argv[++argind];
					else {
						if(opterr) fprintf(stderr, "%s: option requires an argument -- '%c'\n", argv[0], optopt);
						return getopt_missing_arg(optstring);
					}
				}
				optind = argind + 1;
				nextchar = NULL;
			} else optarg = NULL;
			optind = (nextchar && *nextchar) ? argind : argind + 1;
			return optopt;
		}
		if(long_only) {
			if(opterr) fprintf(stderr, "%s: unrecognized option `-%s'\n", argv[0], --nextchar);
			nextchar = NULL;
			optopt = 0;
		} else if(opterr) {
			fprintf(stderr, "%s: invalid option -- %c\n", argv[0], optopt);
		}
		optind = (nextchar && *nextchar) ? argind : argind + 1;
		return '?';
	}
	if(optmark > optbase) {
		int optspan = argind - optmark + 1;
		char *this_arg[optspan];
		char **arglist = (char **)argv;		// 447
		int i;

		// Save
		for(i = 0; i < optspan; i++) {
			this_arg[i] = arglist[optmark + i];
		}

		// Move
		for(optmark--; optmark >= optbase; optmark--) {
			arglist[optmark + optspan] = arglist[optmark];
		}

		// Restore
		for(i = 0; i < optspan; i++) {
			arglist[optbase + i] = this_arg[i];
		}

		optbase += optspan;
	} else optbase = argind + 1;

	while(argc > ++argind) {
		nextchar = argv[optmark = argind];
		if(*nextchar == '-' && *++nextchar) {
			if(*nextchar == '-') {
				char *t = nextchar + 1;
				if(*t) {
					// A long option ?
					if(long_opts) nextchar = t;
				} else {
					// End of option mark
					if(optmark > optbase) {
						char *this_arg = argv[optmark];
						char **arglist = (char **)argv;		// 517
						do {
							arglist[optmark] = arglist[optmark - 1];
						} while(optmark-- > optbase);
						arglist[optbase] = this_arg;
					}
					optind = ++optbase;
					return -1;
				}
			} else if(!long_only) {
				long_opts = NULL;
			}

			if(long_opts) {
				int matched = 1;
				int i;
				optarg = NULL;
				for(i = 0; long_opts && long_opts[i].name; i++) switch(getopt_match_long(nextchar, long_opts[i].name)) {
					case GETOPT_LONG_EXACT_MATCH:
						nextchar = NULL;
						return getopt_resolved(argc, argv, &argind, long_opts, i, index, optstring);
					case GETOPT_LONG_ABBREVIATED_MATCH:
						if(matched >= 0) {
							optopt = 0;
							nextchar = NULL;
							optind = argind + 1;
							if(opterr) fprintf(stderr, "%s: option '%s' is ambiguous\n", argv[0], argv[argind]);
							return '?';
						}
						matched = i;
				}
				if(matched >= 0) {
					nextchar = NULL;
					return getopt_resolved(argc, argv, &argind, long_opts, matched, index, optstring);
				}
				if(!long_only) {
					optopt = 0;
					nextchar = NULL;
					optind = argind + 1;
					if(opterr) fprintf(stderr, "%s: unrecognized option '%s'\n", argv[0], argv[argind]);
					return '?';
				}
			}
			if(*nextchar) {
				return getopt_parse(argc, argv, optstring, NULL, NULL, 0);
			}
		}
		if(*optstring == '-') {
			nextchar = NULL;
			optind = argind + 1;
			optarg = argv[optind];
			return 1;
		}
		if(getopt_conventions(*optstring) & GETOPT_POSIXLY_CORRECT) break;
	}

	optind = optbase;
	return -1;
}

int getopt(int argc, char *const *argv, const char *optstring) {
	return getopt_parse(argc, argv, optstring, NULL, NULL, 0);
}

int getopt_long(int argc, char * const *argv, const char *optstring, const struct option *longopts, int *longindex) {
	return getopt_parse(argc, argv, optstring, longopts, longindex, 0);
}

int getopt_long_only(int argc, char * const *argv, const char *optstring, const struct option *longopts, int *longindex) {
	return getopt_parse(argc, argv, optstring, longopts, longindex, 1);
}
