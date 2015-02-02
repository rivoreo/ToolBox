# 1 "printenv.c"
# 1 "<built-in>"
# 1 "<command-line>"
# 1 "config/wcedef.h" 1
# 1 "<command-line>" 2
# 1 "printenv.c"
# 1 "printenv_u.c" 1
# 1 "/host/arm-mingw32ce-gcc/4.4.0/bin/../lib/gcc/arm-mingw32ce/4.4.0/../../../../arm-mingw32ce/include/stdio.h" 1 3
# 19 "/host/arm-mingw32ce-gcc/4.4.0/bin/../lib/gcc/arm-mingw32ce/4.4.0/../../../../arm-mingw32ce/include/stdio.h" 3
# 1 "/host/arm-mingw32ce-gcc/4.4.0/bin/../lib/gcc/arm-mingw32ce/4.4.0/../../../../arm-mingw32ce/include/_mingw.h" 1 3
# 31 "/host/arm-mingw32ce-gcc/4.4.0/bin/../lib/gcc/arm-mingw32ce/4.4.0/../../../../arm-mingw32ce/include/_mingw.h" 3
       
# 32 "/host/arm-mingw32ce-gcc/4.4.0/bin/../lib/gcc/arm-mingw32ce/4.4.0/../../../../arm-mingw32ce/include/_mingw.h" 3
# 20 "/host/arm-mingw32ce-gcc/4.4.0/bin/../lib/gcc/arm-mingw32ce/4.4.0/../../../../arm-mingw32ce/include/stdio.h" 2 3






# 1 "/host/arm-mingw32ce-gcc/4.4.0/bin/../lib/gcc/arm-mingw32ce/4.4.0/include/stddef.h" 1 3 4
# 211 "/host/arm-mingw32ce-gcc/4.4.0/bin/../lib/gcc/arm-mingw32ce/4.4.0/include/stddef.h" 3 4
typedef unsigned int size_t;
# 323 "/host/arm-mingw32ce-gcc/4.4.0/bin/../lib/gcc/arm-mingw32ce/4.4.0/include/stddef.h" 3 4
typedef short unsigned int wchar_t;
# 352 "/host/arm-mingw32ce-gcc/4.4.0/bin/../lib/gcc/arm-mingw32ce/4.4.0/include/stddef.h" 3 4
typedef short unsigned int wint_t;
# 27 "/host/arm-mingw32ce-gcc/4.4.0/bin/../lib/gcc/arm-mingw32ce/4.4.0/../../../../arm-mingw32ce/include/stdio.h" 2 3

# 1 "/host/arm-mingw32ce-gcc/4.4.0/bin/../lib/gcc/arm-mingw32ce/4.4.0/include/stdarg.h" 1 3 4
# 40 "/host/arm-mingw32ce-gcc/4.4.0/bin/../lib/gcc/arm-mingw32ce/4.4.0/include/stdarg.h" 3 4
typedef __builtin_va_list __gnuc_va_list;
# 29 "/host/arm-mingw32ce-gcc/4.4.0/bin/../lib/gcc/arm-mingw32ce/4.4.0/../../../../arm-mingw32ce/include/stdio.h" 2 3






# 1 "/host/arm-mingw32ce-gcc/4.4.0/bin/../lib/gcc/arm-mingw32ce/4.4.0/../../../../arm-mingw32ce/include/stdlib.h" 1 3
# 21 "/host/arm-mingw32ce-gcc/4.4.0/bin/../lib/gcc/arm-mingw32ce/4.4.0/../../../../arm-mingw32ce/include/stdlib.h" 3
# 1 "/host/arm-mingw32ce-gcc/4.4.0/bin/../lib/gcc/arm-mingw32ce/4.4.0/../../../../arm-mingw32ce/include/stdio.h" 1 3
# 22 "/host/arm-mingw32ce-gcc/4.4.0/bin/../lib/gcc/arm-mingw32ce/4.4.0/../../../../arm-mingw32ce/include/stdlib.h" 2 3






# 1 "/host/arm-mingw32ce-gcc/4.4.0/bin/../lib/gcc/arm-mingw32ce/4.4.0/include/stddef.h" 1 3 4
# 29 "/host/arm-mingw32ce-gcc/4.4.0/bin/../lib/gcc/arm-mingw32ce/4.4.0/../../../../arm-mingw32ce/include/stdlib.h" 2 3
# 78 "/host/arm-mingw32ce-gcc/4.4.0/bin/../lib/gcc/arm-mingw32ce/4.4.0/../../../../arm-mingw32ce/include/stdlib.h" 3
extern int _argc;
extern char** _argv;
# 112 "/host/arm-mingw32ce-gcc/4.4.0/bin/../lib/gcc/arm-mingw32ce/4.4.0/../../../../arm-mingw32ce/include/stdlib.h" 3
extern int __argc;
extern char** __argv;
# 130 "/host/arm-mingw32ce-gcc/4.4.0/bin/../lib/gcc/arm-mingw32ce/4.4.0/../../../../arm-mingw32ce/include/stdlib.h" 3
   int __mb_cur_max (void);
# 315 "/host/arm-mingw32ce-gcc/4.4.0/bin/../lib/gcc/arm-mingw32ce/4.4.0/../../../../arm-mingw32ce/include/stdlib.h" 3
extern __attribute__ ((__dllimport__)) int _fmode;
# 328 "/host/arm-mingw32ce-gcc/4.4.0/bin/../lib/gcc/arm-mingw32ce/4.4.0/../../../../arm-mingw32ce/include/stdlib.h" 3
 double __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) atof (const char*);
 int __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) atoi (const char*);
 long __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) atol (const char*);







double __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) __strtod (const char*, char**);



static

__inline__ double __attribute__((__cdecl__)) __attribute__ ((__nothrow__))
strtod (const char* __restrict__ __nptr, char** __restrict__ __endptr)
{
  return __strtod(__nptr, __endptr);
}
float __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) strtof (const char * __restrict__, char ** __restrict__);
long double __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) strtold (const char * __restrict__, char ** __restrict__);




 long __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) strtol (const char*, char**, int);
 unsigned long __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) strtoul (const char*, char**, int);



 long __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) wcstol (const wchar_t*, wchar_t**, int);
 unsigned long __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) wcstoul (const wchar_t*, wchar_t**, int);
 double __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) wcstod (const wchar_t*, wchar_t**);

float __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) wcstof( const wchar_t * __restrict__, wchar_t ** __restrict__);
long double __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) wcstold (const wchar_t * __restrict__, wchar_t ** __restrict__);
# 379 "/host/arm-mingw32ce-gcc/4.4.0/bin/../lib/gcc/arm-mingw32ce/4.4.0/../../../../arm-mingw32ce/include/stdlib.h" 3
 size_t __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) wcstombs (char*, const wchar_t*, size_t);




 size_t __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) mbstowcs (wchar_t*, const char*, size_t);




 int __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) rand (void);
 void __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) srand (unsigned int);

 void* __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) calloc (size_t, size_t) __attribute__ ((__malloc__));
 void* __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) malloc (size_t) __attribute__ ((__malloc__));
 void* __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) realloc (void*, size_t);
 void __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) free (void*);
 void __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) abort (void) __attribute__ ((__noreturn__));
 void __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) exit (int) __attribute__ ((__noreturn__));


int __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) atexit (void (*)(void));







 void* __attribute__((__cdecl__)) bsearch (const void*, const void*, size_t, size_t,
          int (*)(const void*, const void*));
 void __attribute__((__cdecl__)) qsort(void*, size_t, size_t,
      int (*)(const void*, const void*));

 int __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) abs (int) __attribute__ ((__const__));
 long __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) labs (long) __attribute__ ((__const__));
# 424 "/host/arm-mingw32ce-gcc/4.4.0/bin/../lib/gcc/arm-mingw32ce/4.4.0/../../../../arm-mingw32ce/include/stdlib.h" 3
typedef struct { int quot, rem; } div_t;
typedef struct { long quot, rem; } ldiv_t;

 div_t __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) div (int, int) __attribute__ ((__const__));
 ldiv_t __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) ldiv (long, long) __attribute__ ((__const__));
# 443 "/host/arm-mingw32ce-gcc/4.4.0/bin/../lib/gcc/arm-mingw32ce/4.4.0/../../../../arm-mingw32ce/include/stdlib.h" 3
 void __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) _exit (int) __attribute__ ((__noreturn__));



typedef int (* _onexit_t)(void);
_onexit_t __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) _onexit( _onexit_t );






 char* __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) _ecvt (double, int, int*, int*);
 char* __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) _fcvt (double, int, int*, int*);
 char* __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) _gcvt (double, int, char*);







 char* __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) _itoa (int, char*, int);
 char* __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) _ltoa (long, char*, int);
 char* __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) _ultoa(unsigned long, char*, int);
 wchar_t* __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) _itow (int, wchar_t*, int);
 wchar_t* __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) _ltow (long, wchar_t*, int);
 wchar_t* __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) _ultow (unsigned long, wchar_t*, int);


 long long __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) _atoi64(const char *);
# 521 "/host/arm-mingw32ce-gcc/4.4.0/bin/../lib/gcc/arm-mingw32ce/4.4.0/../../../../arm-mingw32ce/include/stdlib.h" 3
 unsigned int __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) _rotl(unsigned int, int) __attribute__ ((__const__));
 unsigned int __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) _rotr(unsigned int, int) __attribute__ ((__const__));
 unsigned long __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) _lrotl(unsigned long, int) __attribute__ ((__const__));
 unsigned long __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) _lrotr(unsigned long, int) __attribute__ ((__const__));
# 533 "/host/arm-mingw32ce-gcc/4.4.0/bin/../lib/gcc/arm-mingw32ce/4.4.0/../../../../arm-mingw32ce/include/stdlib.h" 3
 char* __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) itoa (int, char*, int);
 char* __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) ltoa (long, char*, int);


 char* __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) ecvt (double, int, int*, int*);
 char* __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) fcvt (double, int, int*, int*);
 char* __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) gcvt (double, int, char*);
# 550 "/host/arm-mingw32ce-gcc/4.4.0/bin/../lib/gcc/arm-mingw32ce/4.4.0/../../../../arm-mingw32ce/include/stdlib.h" 3
void __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) _Exit(int) __attribute__ ((__noreturn__));

extern __inline__ __attribute__((__always_inline__)) void __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) _Exit(int __status)
 { _exit (__status); }


typedef struct { long long quot, rem; } lldiv_t;

lldiv_t __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) lldiv (long long, long long) __attribute__ ((__const__));

long long __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) llabs(long long);
extern __inline__ __attribute__((__always_inline__)) long long __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) llabs(long long _j)
  {return (_j >= 0 ? _j : -_j);}

long long __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) strtoll (const char* __restrict__, char** __restrict, int);
unsigned long long __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) strtoull (const char* __restrict__, char** __restrict__, int);


extern __inline__ __attribute__((__always_inline__)) long long __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) atoll (const char * _c)
 { return _atoi64 (_c); }
# 36 "/host/arm-mingw32ce-gcc/4.4.0/bin/../lib/gcc/arm-mingw32ce/4.4.0/../../../../arm-mingw32ce/include/stdio.h" 2 3
# 142 "/host/arm-mingw32ce-gcc/4.4.0/bin/../lib/gcc/arm-mingw32ce/4.4.0/../../../../arm-mingw32ce/include/stdio.h" 3
typedef void FILE;
# 172 "/host/arm-mingw32ce-gcc/4.4.0/bin/../lib/gcc/arm-mingw32ce/4.4.0/../../../../arm-mingw32ce/include/stdio.h" 3
 FILE* __attribute__((__cdecl__)) _getstdfilex(int);
# 201 "/host/arm-mingw32ce-gcc/4.4.0/bin/../lib/gcc/arm-mingw32ce/4.4.0/../../../../arm-mingw32ce/include/stdio.h" 3
 FILE* __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) fopen (const char*, const char*);
 FILE* __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) freopen (const char*, const char*, FILE*);
 int __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) fflush (FILE*);
 int __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) fclose (FILE*);




 int __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) rename (const char*, const char*);






 char* __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) _tempnam (const char*, const char*);



 int __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) _unlink (const char*);


 char* __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) tempnam (const char*, const char*);



 int __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) unlink (const char*);



 int __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) setvbuf (FILE*, char*, int, size_t);
# 246 "/host/arm-mingw32ce-gcc/4.4.0/bin/../lib/gcc/arm-mingw32ce/4.4.0/../../../../arm-mingw32ce/include/stdio.h" 3
extern int __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) __mingw_fprintf(FILE*, const char*, ...);
extern int __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) __mingw_printf(const char*, ...);
extern int __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) __mingw_sprintf(char*, const char*, ...);
extern int __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) __mingw_snprintf(char*, size_t, const char*, ...);
extern int __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) __mingw_vfprintf(FILE*, const char*, __gnuc_va_list);
extern int __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) __mingw_vprintf(const char*, __gnuc_va_list);
extern int __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) __mingw_vsprintf(char*, const char*, __gnuc_va_list);
extern int __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) __mingw_vsnprintf(char*, size_t, const char*, __gnuc_va_list);
# 335 "/host/arm-mingw32ce-gcc/4.4.0/bin/../lib/gcc/arm-mingw32ce/4.4.0/../../../../arm-mingw32ce/include/stdio.h" 3
 int __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) fprintf (FILE*, const char*, ...);
 int __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) printf (const char*, ...);
 int __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) sprintf (char*, const char*, ...);
 int __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) vfprintf (FILE*, const char*, __gnuc_va_list);
 int __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) vprintf (const char*, __gnuc_va_list);
 int __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) vsprintf (char*, const char*, __gnuc_va_list);
# 350 "/host/arm-mingw32ce-gcc/4.4.0/bin/../lib/gcc/arm-mingw32ce/4.4.0/../../../../arm-mingw32ce/include/stdio.h" 3
 int __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) __msvcrt_fprintf(FILE*, const char*, ...);
 int __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) __msvcrt_printf(const char*, ...);
 int __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) __msvcrt_sprintf(char*, const char*, ...);
 int __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) __msvcrt_vfprintf(FILE*, const char*, __gnuc_va_list);
 int __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) __msvcrt_vprintf(const char*, __gnuc_va_list);
 int __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) __msvcrt_vsprintf(char*, const char*, __gnuc_va_list);





 int __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) _snprintf (char*, size_t, const char*, ...);
 int __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) _vsnprintf (char*, size_t, const char*, __gnuc_va_list);
# 372 "/host/arm-mingw32ce-gcc/4.4.0/bin/../lib/gcc/arm-mingw32ce/4.4.0/../../../../arm-mingw32ce/include/stdio.h" 3
int __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) snprintf (char *, size_t, const char *, ...);
int __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) vsnprintf (char *, size_t, const char *, __gnuc_va_list);

int __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) vscanf (const char * __restrict__, __gnuc_va_list);
int __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) vfscanf (FILE * __restrict__, const char * __restrict__,
       __gnuc_va_list);
int __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) vsscanf (const char * __restrict__,
       const char * __restrict__, __gnuc_va_list);







 int __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) fscanf (FILE*, const char*, ...);
 int __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) scanf (const char*, ...);
 int __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) sscanf (const char*, const char*, ...);




 int __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) fgetc (FILE*);
 char* __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) fgets (char*, int, FILE*);
 int __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) fputc (int, FILE*);
 int __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) fputs (const char*, FILE*);
 char* __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) gets (char*);
 int __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) puts (const char*);
 int __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) ungetc (int, FILE*);
# 452 "/host/arm-mingw32ce-gcc/4.4.0/bin/../lib/gcc/arm-mingw32ce/4.4.0/../../../../arm-mingw32ce/include/stdio.h" 3
extern __inline__ __attribute__((__always_inline__)) int __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) getc (FILE* __F) { return fgetc(__F); }
extern __inline__ __attribute__((__always_inline__)) int __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) putc (int __c, FILE* __F) { return fputc (__c, __F); }
 int __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) getchar (void);
 int __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) putchar(int __c);







 size_t __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) fread (void*, size_t, size_t, FILE*);
 size_t __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) fwrite (const void*, size_t, size_t, FILE*);





 int __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) fseek (FILE*, long, int);
 long __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) ftell (FILE*);
# 510 "/host/arm-mingw32ce-gcc/4.4.0/bin/../lib/gcc/arm-mingw32ce/4.4.0/../../../../arm-mingw32ce/include/stdio.h" 3
typedef long fpos_t;


 int __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) fgetpos (FILE*, fpos_t*);
 int __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) fsetpos (FILE*, const fpos_t*);





 int __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) feof (FILE*);
 int __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) ferror (FILE*);
# 535 "/host/arm-mingw32ce-gcc/4.4.0/bin/../lib/gcc/arm-mingw32ce/4.4.0/../../../../arm-mingw32ce/include/stdio.h" 3
 void __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) clearerr (FILE*);
 void __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) perror (const char*);
# 554 "/host/arm-mingw32ce-gcc/4.4.0/bin/../lib/gcc/arm-mingw32ce/4.4.0/../../../../arm-mingw32ce/include/stdio.h" 3
 int __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) _flushall (void);




 FILE* __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) _fdopen (int, const char*);






 void * __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) _fileno (FILE*);



 int __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) _fcloseall (void);
# 587 "/host/arm-mingw32ce-gcc/4.4.0/bin/../lib/gcc/arm-mingw32ce/4.4.0/../../../../arm-mingw32ce/include/stdio.h" 3
 FILE* __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) fdopen (int, const char*);







 void * __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) fileno (FILE*);
# 636 "/host/arm-mingw32ce-gcc/4.4.0/bin/../lib/gcc/arm-mingw32ce/4.4.0/../../../../arm-mingw32ce/include/stdio.h" 3
 int __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) fwprintf (FILE*, const wchar_t*, ...);
 int __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) wprintf (const wchar_t*, ...);
 int __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) _snwprintf (wchar_t*, size_t, const wchar_t*, ...);
 int __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) vfwprintf (FILE*, const wchar_t*, __gnuc_va_list);
 int __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) vwprintf (const wchar_t*, __gnuc_va_list);
 int __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) _vsnwprintf (wchar_t*, size_t, const wchar_t*, __gnuc_va_list);
 int __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) fwscanf (FILE*, const wchar_t*, ...);
 int __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) wscanf (const wchar_t*, ...);
 int __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) swscanf (const wchar_t*, const wchar_t*, ...);
 wint_t __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) fgetwc (FILE*);
 wint_t __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) fputwc (wchar_t, FILE*);
 wint_t __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) ungetwc (wchar_t, FILE*);



 int __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) swprintf (wchar_t*, const wchar_t*, ...);
 int __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) vswprintf (wchar_t*, const wchar_t*, __gnuc_va_list);



 wchar_t* __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) fgetws (wchar_t*, int, FILE*);
 int __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) fputws (const wchar_t*, FILE*);
 wint_t __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) getwchar (void);
 wint_t __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) putwchar (wint_t);

 wchar_t* __attribute__((__cdecl__)) _getws (wchar_t*);
 int __attribute__((__cdecl__)) _putws (const wchar_t*);
 FILE* __attribute__((__cdecl__)) _wfdopen(int, const wchar_t *);
 FILE* __attribute__((__cdecl__)) _wfopen (const wchar_t*, const wchar_t*);
 FILE* __attribute__((__cdecl__)) _wfreopen (const wchar_t*, const wchar_t*, FILE*);
# 689 "/host/arm-mingw32ce-gcc/4.4.0/bin/../lib/gcc/arm-mingw32ce/4.4.0/../../../../arm-mingw32ce/include/stdio.h" 3
 wchar_t* __attribute__((__cdecl__)) _wtempnam (const wchar_t*, const wchar_t*);
extern __inline__ __attribute__((__always_inline__)) wint_t __attribute__((__cdecl__)) getwc(FILE* f) { return fgetwc(f); }
extern __inline__ __attribute__((__always_inline__)) wint_t __attribute__((__cdecl__)) putwc(wint_t c, FILE* f) { return fputwc(c, f); }



int __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) snwprintf (wchar_t* s, size_t n, const wchar_t* format, ...);
extern __inline__ __attribute__((__always_inline__)) int __attribute__((__cdecl__)) __attribute__ ((__nothrow__))
vsnwprintf (wchar_t* s, size_t n, const wchar_t* format, __gnuc_va_list arg)
  { return _vsnwprintf ( s, n, format, arg);}
int __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) vwscanf (const wchar_t * __restrict__, __gnuc_va_list);
int __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) vfwscanf (FILE * __restrict__,
         const wchar_t * __restrict__, __gnuc_va_list);
int __attribute__((__cdecl__)) __attribute__ ((__nothrow__)) vswscanf (const wchar_t * __restrict__,
         const wchar_t * __restrict__, __gnuc_va_list);
# 2 "printenv_u.c" 2


extern char ** environ;

int printenv_main (int argc, char **argv)
{
    char** e;
    char* v;
    int i;

    if (argc == 1) {
        e = environ;
        while (*e) {
            printf("%s\n", *e);
            e++;
        }
    } else {
        for (i=1; i<argc; i++) {
            v = getenv(argv[i]);
            if (v) {
                printf("%s\n", v);
            }
        }
    }

    return 0;
}
# 2 "printenv.c" 2

int main(int argc, char *argv[]){
 return printenv_main(argc, argv);
}
