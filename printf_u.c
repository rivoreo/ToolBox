/*
 * Copyright (c) 1989, 1993
 *	The Regents of the University of California.  All rights reserved.
 * Copyright (c) 1997-2005
 *	Herbert Xu <herbert@gondor.apana.org.au>.  All rights reserved.
 *
 * Copyright 2015-2017 Rivoreo
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

#include <sys/types.h>
#include <sys/param.h>
#include <ctype.h>
#include <errno.h>
#ifdef __INTERIX
#include <stdint.h>
extern long long int strtoq(const char *, char **, int);
extern unsigned long long int strtouq(const char *, char **, int);
#define strtoimax strtoq
#define strtoumax strtouq
#else
#include <inttypes.h>
#endif
#include <limits.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

#ifdef BSD
#include <err.h>
#else
#define warnx(F,...) fprintf(stderr,(F"\n"),##__VA_ARGS__)
#endif

static int	 getchr(void);
static double	 getdouble(void);
static intmax_t	 getintmax(void);
static uintmax_t getuintmax(void);
static char	*getstr(void);
static char	*mklong(const char *, const char *);
static void      check_conversion(const char *, const char *);

static char nullstr[1] = { 0 };
static int	rval;
static char  **gargv;

#define isodigit(c) ((c) >= '0' && (c) <= '7')
#define octtobin(c) ((c) - '0')
#define hextobin(c) ((c) - (isalpha(c) ? (islower(c) ? 87 : 55) : '0'))

#define PF(f, func) \
switch ((char *)param - (char *)array) { \
	default: \
		(void)printf(f, array[0], array[1], func); \
		break; \
	case sizeof(*param): \
		(void)printf(f, array[0], func); \
		break; \
	case 0: \
		(void)printf(f, func); \
		break; \
}

/*
 * Print "standard" escape characters 
 */
static char *
conv_escape(const char *str, int *conv_ch)
{
	int value;
	int ch;

	ch = *str;

	switch (ch) {
		default:
		case 0:
			value = '\\';
			goto out;

		case '0': case '1': case '2': case '3':
		case '4': case '5': case '6': case '7':
			ch = 3;
			value = 0;
			do {
				value <<= 3;
				value += octtobin(*str++);
			} while (isodigit(*str) && --ch);
			goto out;

		case 'x':
			if(!*++str || !isxdigit(*str)) {
				fprintf(stderr, "missing hex digit for \\x\n");
				str--;
				value = '\\';
				goto out;
			}
			value = hextobin(*str);
			if(*++str && isxdigit(*str)) {
				value *= 16;
				value += hextobin(*str);
			}
			break;

		case '\\':	value = '\\';	break;	/* backslash */
		case 'a':	value = '\a';	break;	/* alert */
		case 'b':	value = '\b';	break;	/* backspace */
		case 'f':	value = '\f';	break;	/* form-feed */
		case 'n':	value = '\n';	break;	/* newline */
		case 'r':	value = '\r';	break;	/* carriage-return */
		case 't':	value = '\t';	break;	/* tab */
		case 'v':	value = '\v';	break;	/* vertical-tab */
	}

	str++;
out:
	*conv_ch = value;
	return (char *)str;
}

static char *conv_escape_str(const char *str, int *done)
{
	//fprintf(stderr, "function: conv_escape_str(%p<%s>, %p)\n", str, str, done);
	int ch;
	char *buffer = NULL;
	size_t buffer_size = 0, i = 0;

	*done = 0;
	do {
		if(buffer_size <= i) {
			buffer = realloc(buffer, buffer_size += 10);
			if(!buffer) return NULL;
		}

		ch = *str++;
		if (ch != '\\')
			continue;

		ch = *str++;
		if (ch == 'c') {
			/* \c as in SYSV echo - abort all processing.... */
			//ch = 0x100;
			*done = 1;
			buffer[i] = 0;
			break;
		}

		/* 
		 * %b string octal constants are not like those in C.
		 * They start with a \0, and are followed by 0, 1, 2, 
		 * or 3 octal digits. 
		 */
		if (ch == '0') {
			unsigned char i;
			i = 3;
			ch = 0;
			do {
				unsigned k = octtobin(*str);
				if (k > 7)
					break;
				str++;
				ch <<= 3;
				ch += k;
			} while (--i);
			continue;
		}

		/* Finally test for sequences valid in the format string */
		str = conv_escape(str - 1, &ch);
	} while((buffer[i++] = ch));
	return buffer;
}

static char *
mklong(const char *str, const char *ch)
{
	char *copy;
	size_t len;	

	len = ch - str + 3;
	copy = malloc(len);
	if(!copy) return NULL;
	memcpy(copy, str, len - 3);
	copy[len - 3] = 'j';
	copy[len - 2] = *ch;
	copy[len - 1] = '\0';
	return copy;	
}

static int
getchr(void)
{
	int val = 0;

	if(*gargv) val = **gargv++;
	return val;
}

static char *
getstr(void)
{
	char *val = nullstr;

	if(*gargv) val = *gargv++;
	return val;
}

static intmax_t
getintmax(void)
{
	intmax_t val = 0;
	char *cp, *ep;

	cp = *gargv;
	if(!cp) return 0;
	gargv++;
	if(*cp == '\"' || *cp == '\'') return (unsigned char)cp[1];
	errno = 0;
	val = strtoimax(cp, &ep, 0);
	check_conversion(cp, ep);
	return val;
}

static uintmax_t
getuintmax(void)
{
	uintmax_t val = 0;
	char *cp, *ep;

	cp = *gargv;
	if(!cp) return 0;
	gargv++;
	if(*cp == '\"' || *cp == '\'') return (unsigned char)cp[1];
	errno = 0;
	val = strtoumax(cp, &ep, 0);
	check_conversion(cp, ep);
	return val;
}

static double
getdouble(void)
{
	double val;
	char *cp, *ep;

	cp = *gargv;
	if(!cp) return 0;
	gargv++;
	if(*cp == '\"' || *cp == '\'') return (unsigned char)cp[1];
	errno = 0;
	val = strtod(cp, &ep);
	check_conversion(cp, ep);
	return val;
}

static void
check_conversion(const char *s, const char *ep)
{
	if (*ep) {
		if (ep == s) warnx("%s: expected numeric value", s);
		else warnx("%s: not completely converted", s);
		rval = 1;
	} else if (errno == ERANGE) {
		warnx("%s: %s", s, strerror(ERANGE));
		rval = 1;
	}
}

int printf_main(int argc, char *argv[])
{
	char *fmt;
	char *format;
	int ch;
	//int have_format = 0;

	rval = 0;

	if(argc > 1 && argv[1][0] == '-') switch(argv[1][1]) {
		case 0:
			break;
		case '-':
			if(argv[1][2]) {
				fprintf(stderr, "%s: Unknown option '%s'\n", argv[0], argv[1]);
				return -1;
			}
			argv[1] = argv[0];
			argv++;
			argc--;
			break;
		default:
			fprintf(stderr, "%s: Unknown option '-%c'\n", argv[0], argv[1][1]);
			return -1;
	}
	format = argv[1];

	if (!format) {
		fprintf(stderr, "Usage: %s <format> [<arg>] [...]\n", argv[0]);
		return -1;
	}

	gargv = argv + 2;

#define SKIP1	"#-+ 0"
#define SKIP2	"*0123456789"
	do {
		/*
		 * Basic algorithm is to scan the format string for conversion
		 * specifications -- once one is found, find out if the field
		 * width or precision is a '*'; if it is, gather up value. 
		 * Note, format strings are reused as necessary to use up the
		 * provided arguments, arguments of zero/null string are 
		 * provided to use up the format string.
		 */

		/* find next format specification */
		for (fmt = format; (ch = *fmt++) ;) {
			char *start;
			char nextch;
			int array[2];
			int *param;

			if (ch == '\\') {
				int c_ch;
				fmt = conv_escape(fmt, &c_ch);
				ch = c_ch;
				goto pc;
			}
			if (ch != '%' || (*fmt == '%' && (++fmt || 1))) {
pc:
				putchar(ch);
				continue;
			}

			/* Ok - we've found a format specification,
			   Save its address for a later printf(). */
			//have_format = 1;
			start = fmt - 1;
			param = array;

			/* skip to field width */
			fmt += strspn(fmt, SKIP1);
			if (*fmt == '*')
				*param++ = getintmax();

			/* skip to possible '.', get following precision */
			fmt += strspn(fmt, SKIP2);
			if (*fmt == '.')
				++fmt;
			if (*fmt == '*')
				*param++ = getintmax();

			fmt += strspn(fmt, SKIP2);

			ch = *fmt;
			if (!ch) {
				warnx("missing format character");
				return 1;
			}
			/* null terminate format string to we can use it
			   as an argument to printf. */
			nextch = fmt[1];
			fmt[1] = 0;
			switch (ch) {

				case 'b': {
					int done;
					char *p = conv_escape_str(getstr(), &done);
					//char *p = stackblock();
					if(!p) {
						perror(argv[0]);
						return 1;
					}
					*fmt = 's';
					PF(start, p);
					free(p);
					//fprintf(stderr, "done = %d\n", done);
					/* escape if a \c was encountered */
					if(done) return rval;
					*fmt = 'b';
					break;
				}
				case 'c': {
					int p = getchr();
					PF(start, p);
					break;
				}
				case 's': {
					char *p = getstr();
					PF(start, p);
					break;
				}
				case 'd':
				case 'i': {
					intmax_t p = getintmax();
					char *f = mklong(start, fmt);
					if(!f) {
						perror(argv[0]);
						return 1;
					}
					PF(f, p);
					free(f);
					break;
				}
				case 'o':
				case 'u':
				case 'x':
				case 'X': {
					uintmax_t p = getuintmax();
					char *f = mklong(start, fmt);
					if(!f) {
						perror(argv[0]);
						return 1;
					}
					PF(f, p);
					free(f);
					break;
				}
				case 'e':
				case 'E':
				case 'f':
				case 'g':
				case 'G': {
					double p = getdouble();
					PF(start, p);
					break;
				}
				default:
					warnx("%s: invalid directive", start);
					return 1;
			}
			*++fmt = nextch;
		}
	} while (gargv != argv + 2 && *gargv);

	return rval;
}

