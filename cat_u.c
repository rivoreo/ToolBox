/* $NetBSD: cat.c,v 1.43 2004/01/04 03:31:28 jschauma Exp $	*/

/*
 * Copyright (c) 1989, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Kevin Fall.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <sys/param.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef _WIN32
#ifdef _WIN32_WCE
#define CAT_BUFSIZ (1024)
#else
#define CAT_BUFSIZ (2048)
#endif
#else
#define CAT_BUFSIZ (4096)
#endif

#ifdef _WIN32
#define FLAGS "benstvT"
#else
#define FLAGS "beflnstvT"
#endif

static int bflag, eflag, nflag, sflag, tflag, vflag;
#ifndef _WIN32
static int fflag, lflag;
#endif
static int Tflag;
static int rval;
static const char *filename;

static void cook_buf(FILE *fp) {
	int ch, gobble, line, prev;
	int stdout_err = 0;

	line = gobble = 0;
	for (prev = '\n'; (ch = getc(fp)) != EOF; prev = ch) {
		if (prev == '\n') {
			if (ch == '\n') {
				if(sflag) {
					if(!gobble && putchar(ch) == EOF) break;
					gobble = 1;
					continue;
				}
				if(nflag) {
					if(!bflag) {
						if(fprintf(stdout, "%6d\t", ++line) < 0) {
							stdout_err++;
							break;
						}
					} else if(eflag) {
						if(fprintf(stdout, "%6s\t", "") < 0) {
							stdout_err++;
							break;
						}
					}
				}
			} else if(nflag) {
				if(fprintf(stdout, "%6d\t", ++line) < 0) {
					stdout_err++;
					break;
				}
			}
		}
		gobble = 0;
		if(ch == '\n') {
			if(eflag && putchar('$') == EOF) break;
		} else if (ch == '\t') {
			if (tflag) {
				if (putchar('^') == EOF || putchar('I') == EOF) break;
				continue;
			}
		} else if (vflag) {
			if (!isascii(ch)) {
				if (putchar('M') == EOF || putchar('-') == EOF) break;
				ch = (ch) & 0x7f;
			}
			if (iscntrl(ch)) {
				if(putchar('^') == EOF ||
				putchar(ch == '\177' ? '?' : ch | 0100) == EOF) break;

				continue;
			}
		}
		if(putchar(ch) == EOF) break;
	}
	if (stdout_err) {
		perror(filename);
		rval = 1;
	}
}

static void cook_args(char **argv) {
	FILE *fp;

	fp = stdin;
	filename = "stdin";
	do {
		if (*argv) {
			if(strcmp(*argv, "-") == 0) fp = stdin;
			else if((fp = fopen(*argv,
#ifndef _WIN32
			fflag ? "rf" : 
#endif
			"r")) == NULL) {
				perror("fopen");
				rval = 1;
				argv++;
				continue;
			}
			filename = *argv++;
		}
		cook_buf(fp);
		if(fp != stdin) fclose(fp);
	} while(*argv);
}

static void raw_cat(int rfd) {
	static char *buf;
	static char fb_buf[CAT_BUFSIZ];
	static size_t bsize;
#ifndef _WIN32
	struct stat sbuf;
#endif
	ssize_t nr, nw, off;
	int wfd;
	struct timeval tv;
	double oldtime = 0, newtime;

	wfd = fileno(stdout);
#if !defined _WIN32 || defined _WIN32_WNT_NATIVE
	if(!buf) {
		if(fstat(wfd, &sbuf) == 0) {
			bsize = sbuf.st_blksize > CAT_BUFSIZ ? sbuf.st_blksize : CAT_BUFSIZ;
			buf = malloc(bsize);
		}
#endif
		if(!buf) {
			buf = fb_buf;
			bsize = CAT_BUFSIZ;
		}
#if !defined _WIN32 || defined _WIN32_WNT_NATIVE
	}
#endif
	if(Tflag) {
		if(gettimeofday(&tv, NULL) < 0) {
			fprintf(stderr, "gettimeofday failed: %s, disabling timing output\n", strerror(errno));
			Tflag = 0;
		} else {
			oldtime = tv.tv_sec + (double)tv.tv_usec / 1000000;
		}
	}
	while(1) {
		/* Older util-linux script(1) erroneously measuring the time
		 * before calling read(2), this causes multiple issues
		 * including the first timing data being wasted, the timing
		 * data has to be read off-by-one when replaying, and missing
		 * timing data of the last chunk of data. This bug has been
		 * fixed in newer util-linux by commits b0d6b85 and a21f7ec;
		 * it however hopelessly broke the compatibily with existing
		 * recordings, therefore we have to keep the old buggy
		 * behavior, while additionally printing the timing data for
		 * last chunk of data after EOF. */
		if(Tflag && gettimeofday(&tv, NULL) < 0) {
			fprintf(stderr, "gettimeofday failed: %s, disabling timing output\n", strerror(errno));
			Tflag = 0;
		}
		nr = read(rfd, buf, bsize);
		if(nr <= 0) break;
		if(Tflag) {
			newtime = tv.tv_sec + (double)tv.tv_usec / 1000000;
			fprintf(stderr, "%f %zd\n", newtime - oldtime, nr);
			oldtime = newtime;
		}
		for(off = 0; nr > 0; nr -= nw, off += nw) {
			if((nw = write(wfd, buf + off, (size_t)nr)) < 0) {
				perror("write");
				exit(EXIT_FAILURE);
			}
		}
	}
	// Print timing data for last chunk
	if(Tflag && gettimeofday(&tv, NULL) == 0) {
		newtime = tv.tv_sec + (double)tv.tv_usec / 1000000;
		fprintf(stderr, "%f 0\n", newtime - oldtime);
	}
	if(nr < 0) {
		fprintf(stderr,"%s: invalid length\n", filename);
		rval = 1;
	}
}

static void raw_args(char **argv) {
	int fd;

	fd = fileno(stdin);
	filename = "stdin";
	do {
		if(*argv) {
			if(strcmp(*argv, "-") == 0) fd = fileno(stdin);
#ifndef _WIN32
			else if(fflag) {
				struct stat st;
				fd = open(*argv, O_RDONLY|O_NONBLOCK);
				if(fd == -1) goto skip;

				if(fstat(fd, &st) == -1) {
					close(fd);
					goto skip;
				}
				if(!S_ISREG(st.st_mode)) {
					close(fd);
					errno = EINVAL;
					goto skipnomsg;
				}
			} else
#endif
			if((fd = open(*argv, O_RDONLY)) == -1) {
#ifndef _WIN32
skip:
#endif
				perror(*argv);
#ifndef _WIN32
skipnomsg:
#endif
				rval = 1;
				++argv;
				continue;
			}
			filename = *argv++;
		}
		raw_cat(fd);
		if(fd != fileno(stdin)) close(fd);
	} while(*argv);
}

int cat_main(int argc, char *argv[]) {
	int ch;
#ifndef _WIN32
	struct flock stdout_lock;
#endif
	while((ch = getopt(argc, argv, FLAGS "h")) != -1) switch (ch) {
		case 'b':
			bflag = nflag = 1;	/* -b implies -n */
			break;
		case 'e':
			eflag = vflag = 1;	/* -e implies -v */
			break;
#ifndef _WIN32
		case 'f':
			fflag = 1;
			break;
		case 'l':
			lflag = 1;
			break;
#endif
		case 'n':
			nflag = 1;
			break;
		case 's':
			sflag = 1;
			break;
		case 't':
			tflag = vflag = 1;	/* -t implies -v */
			break;
		case 'v':
			vflag = 1;
			break;
		case 'T':
			Tflag = 1;		/* Print timing information to stderr */
			break;
		default:
		case 'h':
			fprintf(stderr, "Usage: cat"
#if defined _WIN32 && !defined _WIN32_WNT_NATIVE
				".exe"
#endif
				" [-" FLAGS "] [-] [<file>] [...]\n");
			return -1;
	}
	argv += optind;
#ifndef _WIN32
	if(lflag) {
		stdout_lock.l_len = 0;
		stdout_lock.l_start = 0;
		stdout_lock.l_type = F_WRLCK;
		stdout_lock.l_whence = SEEK_SET;
		if(fcntl(STDOUT_FILENO, F_SETLKW, &stdout_lock) == -1) {
			perror("fcntl");
			return EXIT_FAILURE;
		}
	}
#endif
	if(bflag || eflag || nflag || sflag || tflag || vflag) cook_args(argv);
	else raw_args(argv);

	if(fflush(stdout)) {
		perror("fflush");
		return EXIT_FAILURE;
	}
	return rval;
}
