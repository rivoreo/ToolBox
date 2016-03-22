# 工具箱的 Makefile

OS_NAME = $(shell uname -s)
OUTFILE = toolbox

ifeq ($(OS_NAME),Darwin)
MAC_HELP = \
	"\nMac OS X compile examples:\n"					\
	"make [unity|separate]\n"						\
	"LDFLAGS=\"-arch i386 -arch x86_64\" CFLAGS=\"-arch i386 -arch x86_64\" make [unity|separate]\n"	\
	"DARWIN=1 CC=\"gcc -arch i386 -arch x86_64\" make [unity|separate]\n"	\
	"DARWIN=1 CC=\"gcc -arch i386 -mmacosx-version-min=10.4\" make [unity|separate]\n"
endif

ifeq ($($(shell echo -e)),-e)
ECHO = echo
else
ECHO = echo -e
endif

ifndef TOUCH
TOUCH = $(shell [ -x /usr/bin/touch ] && echo /usr/bin/touch || echo /bin/touch)
endif

ifeq ($(CC),cc)
CC = gcc
CC_VERSION = $(shell gcc --version | grep -E "gcc.+[0-9]\.[0-9]\.[0-9]" | grep -Eo "[0-9]\.[0-9]" | grep -Eo "\.[0-9]" | sed "2d")
ifeq ($(CC_VERSION),.9)
NEED_LIBPCRE = 1
endif
ifeq ($(OS_NAME),Linux)
LINUX = 1
else
NO_SELINUX = 1
endif
ifeq ($(OS_NAME),Darwin)
DARWIN = 1
endif
ifeq ($(OS_NAME),GNU)
GNU = 1
endif
ifeq ($(OS_NAME),Interix)
INTERIX = 1
endif
ifeq ($(OS_NAME),FreeBSD)
FREEBSD = 1
endif
ifeq ($(OS_NAME),SunOS)
SUNOS = 1
endif
endif

CFLAGS += -Iinclude -O1 -Wall

ifdef DARWIN
LIB_NAME = libtoolbox.dylib
else
LIB_NAME = libtoolbox.so
endif

ifdef SHARED_OBJECT
ifndef INTERIX		# PIC in Interix default gcc is buggy; let him to use relocation
CFLAGS += -fPIC
endif
CFLAGS += -D_SHARED
ifdef DARWIN
LDFLAGS += --shared
else
ifeq ($(SHARED_OBJECT),noexec)
LDFLAGS += --shared
else
# Both shared library and executable
# Option --pie replaces --shared, and passing a -E to the linker to export all symbols
LDFLAGS += --pie -Wl,-E
endif
endif		# DARWIN
OUTFILE = $(LIB_NAME)
NO_STATIC = 1
else
ifndef DARWIN
ifndef SUNOS
ifndef NO_STATIC
UNITY_LDFLAGS = --static
endif
endif
endif		# !NO_STATIC
endif		# SHARED_OBJECT

ALL_TOOLS := \
	cat_u.o \
	chmod_u.o \
	chown_u.o \
	chroot_u.o \
	clear_u.o \
	cmp_u.o \
	date_u.o \
	dd_u.o \
	df_u.o \
	du_u.o \
	exists_u.o \
	getopt_u.o \
	hd_u.o \
	hostname_u.o \
	id_u.o \
	ioctl_u.o \
	kill_u.o \
	ln_u.o \
	ls_u.o \
	lsof_u.o \
	md5_u.o \
	more_u.o \
	mkdir_u.o \
	mknod_u.o \
	mkswap_u.o \
	modexe_u.o \
	modexeb_u.o \
	mtdread_u.o \
	mv_u.o \
	nohup_u.o \
	ps_u.o \
	readlink_u.o \
	readtty_u.o \
	renice_u.o \
	rm_u.o \
	rmdir_u.o \
	schedtop_u.o \
	service_u.o \
	sleep_u.o \
	tee_u.o \
	touch_u.o \
	uptime_u.o \
	which_u.o

ifdef GNUTLS
CRYPT_LIB = -lgnutls-openssl -lgnutls -lgpg-error
else
ifdef NO_OPENSSL
CFLAGS += -D_NO_OPENSSL
ifdef SUNOS
CRYPT_LIB = -lmd5
else
CRYPT_LIB = -lcrypt
endif
else
ifdef SUNOS
# Add lib path /lib for OpenSolaris
CRYPT_LIB = -L/lib -L/usr/lib -lcrypto
else
CRYPT_LIB = -lcrypto
endif
endif	# NO_OPENSSL
endif	# GNUTLS

ifdef NO_UTMPX
CFLAGS += -D_NO_UTMPX
endif

ifdef MINGW
SUFFIX := .exe
CFLAGS += -D_NO_SELINUX
ifeq ($(CC),arm-mingw32ce-gcc)
CFLAGS += --include include/wcedef.h
LIBS += -Lwcelib -lc
TIME_LIB = -lmmtimer
SOCKET_LIB = -lws2
endif
ifeq ($(CC),i586-mingw32msvc-gcc)
LIBS += -Lposix-io-for-windows -lposixio
DEPEND = posix-io-for-windows/libposixio.a
export CONSOLE = 1
TIME_LIB = -lwinmm
SOCKET_LIB = -lws2_32
endif
else
EXTRA_TOOLS := \
	chown \
	chroot \
	dd \
	du \
	id \
	kill \
	ln \
	lsof \
	mknod \
	mtdread \
	nohup \
	ps \
	readlink \
	readtty \
	renice \
	schedtop \
	service

ifdef DARWIN
NO_SELINUX = 1
NO_UTIMENSAT = 1
CFLAGS += -fnested-functions
LIBS += -Lmaclib
NEED_LIBGETOPT = 1
ifndef SHARED_OBJECT
ALL_TOOLS += printenv_u.o
endif
else
ifdef FREEBSD
NO_UTIMENSAT = 1
NEED_LIBGETOPT = 1
else
# Empty, !Windows && !Darwin && !FreeBSD
endif
ALL_TOOLS += \
	printenv_u.o \
	top_u.o
EXTRA_TOOLS += \
	top
TIME_LIB = -lrt
ifdef GNU
NO_SELINUX = 1
# utimensat is not implemented in GNU/Hurd
NO_UTIMENSAT = 1
CFLAGS += "-DPATH_MAX=(512)"
else
ifndef INTERIX
ifndef FREEBSD
ifndef SUNOS
# !Windows && !Darwin && !GNU && !Interix && !FreeBSD && !Solaris
ALL_TOOLS += \
	getevent_u.o \
	insmod_u.o \
	lsmod_u.o \
	netstat_u.o \
	notify_u.o \
	rmmod_u.o \
	rotatefb_u.o \
	route_u.o \
	sendevent_u.o \
	setconsole_u.o \
	setkey_u.o \
	vmstat_u.o
EXTRA_TOOLS +=  \
	getevent \
	insmod \
	lsmod \
	netstat \
	notify \
	rmmod \
	rotatefb \
	route \
	sendevent \
	setconsole \
	setkey \
	vmstat
endif		# !SUNOS
endif		# !FREEBSD
ALL_TOOLS += \
	swapoff_u.o \
	swapon_u.o
EXTRA_TOOLS +=  \
	swapoff \
	swapon
endif		# !INTERIX
endif		# GNU
endif		# DARWIN
ifdef NO_SELINUX
CFLAGS += -D_NO_SELINUX
else
ALL_TOOLS += \
	chcon_u.o \
	getenforce_u.o \
	restorecon_u.o \
	runcon_u.o \
	setenforce_u.o \
	setsebool_u.o
EXTRA_TOOLS += \
	chcon \
	getenforce \
	restorecon \
	runcon \
	setenforce \
	setsebool
SELINUX_LIBS = -lselinux
ifndef NO_STATIC
SELINUX_LIBS += -lsepol
ifdef NEED_LIBPCRE
SELINUX_LIBS += -lpcre
endif
endif		# !NO_STATIC
endif		# NO_SELINUX
endif		# MINGW
BASE_TOOLS := \
	cat$(SUFFIX) \
	chmod$(SUFFIX) \
	clear$(SUFFIX) \
	cmp$(SUFFIX) \
	date$(SUFFIX) \
	df$(SUFFIX) \
	exists$(SUFFIX) \
	getopt$(SUFFIX) \
	hd$(SUFFIX) \
	hostname$(SUFFIX) \
	ioctl$(SUFFIX) \
	link$(SUFFIX) \
	ls$(SUFFIX) \
	md5$(SUFFIX) \
	mkdir$(SUFFIX) \
	mkswap$(SUFFIX) \
	modexe$(SUFFIX) \
	modexeb$(SUFFIX) \
	more$(SUFFIX) \
	mv$(SUFFIX) \
	printenv$(SUFFIX) \
	rm$(SUFFIX) \
	rmdir$(SUFFIX) \
	sleep$(SUFFIX) \
	tee$(SUFFIX) \
	touch$(SUFFIX) \
	unlink$(SUFFIX) \
	uptime$(SUFFIX) \
	which$(SUFFIX)

ifdef INTERIX
NO_UTIMENSAT = 1
CFLAGS += -D_ALL_SOURCE -D_NO_UTIMES
NEED_LIBGETOPT = 1
MATH_LIB = -lm
TIME_LIB =
else
ifdef SUNOS
NO_UTIMENSAT = 1
CFLAGS += -D__EXTENSIONS__ -D_NO_STATFS -D__C99FEATURES__ -std=gnu99
MATH_LIB = -lm
SOCKET_LIB = -lnsl -lsocket
#ifndef NO_OPENSSL
# Add lib path /lib for OpenSolaris
#CRYPT_LIB = -L/lib -lcrypto
#endif
else
ifndef MINGW
ALL_TOOLS += \
	iftop_u.o \
	isptrace1allowed_u.o \
	kill1_u.o
EXTRA_TOOLS += \
	iftop \
	isptrace1allowed \
	kill1
endif
endif		# SUNOS

ALL_TOOLS += \
	dmesg_u.o \
	ifconfig_u.o \
	r_u.o \
	reboot_u.o \
	sync_u.o
BASE_TOOLS += \
	reboot$(SUFFIX)
ifndef MINGW
EXTRA_TOOLS += \
	dmesg \
	ifconfig \
	r \
	sync
endif
endif		# INTERIX

ifdef NO_UTIMENSAT
CFLAGS += -D_NO_UTIMENSAT
endif

ifdef NEED_LIBGETOPT
CFLAGS += -Ilibgetopt
LIBS += -Llibgetopt -lgetopt
DEPEND += libgetopt/libgetopt.a
endif

LDLIBS = $(LIBS)

TRAN_SRC = \
	cat.c \
	chcon.c \
	chown.c \
	chroot.c \
	clear.c \
	cmp.c \
	du.c \
	exists.c \
	getenforce.c \
	getevent.c \
	getopt.c \
	hd.c \
	hostname.c \
	id.c \
	insmod.c \
	ioctl.c \
	isptrace1allowed.c \
	kill.c \
	kill1.c \
	ln.c \
	ls.c \
	md5.c \
	mkdir.c \
	mknod.c \
	mkswap.c \
	modexe.c \
	more.c \
	mv.c \
	netstat.c \
	nohup.c \
	notify.c \
	ps.c \
	tee.c \
	top.c \
	readlink.c \
	readtty.c \
	reboot.c \
	renice.c \
	restorecon.c \
	rmmod.c \
	rotatefb.c \
	route.c \
	runcon.c \
	schedtop.c \
	sendevent.c \
	service.c \
	setconsole.c \
	setenforce.c \
	setkey.c \
	sleep.c \
	swapoff.c \
	swapon.c \
	sync.c \
	touch.c \
	which.c


unity:	$(DEPEND) $(OUTFILE)

separate:	$(DEPEND) $(BASE_TOOLS) $(EXTRA_TOOLS)

ifndef SHARED_OBJECT
$(LIB_NAME):
	SHARED_OBJECT=1 $(MAKE)
endif

$(OUTFILE):	$(ALL_TOOLS) toolbox.o
	$(CC) $(LDFLAGS) $(UNITY_LDFLAGS) $^ -o $@ $(LIBS) $(MATH_LIB) $(SOCKET_LIB) $(SELINUX_LIBS) $(TIME_LIB) $(CRYPT_LIB) -lpthread

#separate-mingw:

%.c:	%_u.c
	[ -f $@ ] && { $(TOUCH) -c $@; exit 0; } || $(SHELL) defmain.sh $*

cleanc:
	/bin/rm -f $(TRAN_SRC)

clean:	cleanc
	/bin/rm -f toolbox toolbox.dll $(LIB_NAME) $(BASE_TOOLS) $(EXTRA_TOOLS) *.o *.exe
#	$(MAKE) -C libgetopt $@
	/bin/rm -f libgetopt/*.o libgetopt/*.a
	$(MAKE) -C posix-io-for-windows distclean

help:
	@echo You should do the clean action \(make clean\) before you change the build mode!
	@echo
	@echo build mode:
	@echo " make unity		Build to a multi-call binary (default)"
	@echo " make separate		Build to multi binaries"
#	@echo " make separate-mingw	Windows only"
#	@echo " make mingw		Same as before"
#	@echo " make separate-mingwce	Windows CE (ARM) only"
#	@echo " make mingwce		Same as before"
	@echo
	@echo other actions:
	@echo " make clean		Clean all output files"
	@echo " make help		This help"
	@echo
	@echo
	@echo cross compile examples:
	@echo " CC=arm-linux-gcc make [unity|separate]			To arm-linux"
	@echo " MINGW=1 CC=arm-mingw32ce-gcc make separate		To arm-windowsce"
	@echo " MINGW=1 CC=i586-mingw32msvc-gcc make separate		To x86-windows"
	@echo " make -f nt.mk [unity|separate]				To x86-windowsnt"
	@$(ECHO) $(MAC_HELP)

cat.exe:	cat.c
	$(CC) $(CFLAGS) $(LDFLAGS) cat.c -o cat.exe $(LIBS)

chcon:	chcon.c
	$(CC) $(CFLAGS) $(LDFLAGS) chcon.c -o $@ $(LIBS) $(SELINUX_LIBS)

chmod.exe:	chmod.c
	$(CC) $(CFLAGS) $(LDFLAGS) chmod.c -o chmod.exe $(LIBS)

cmp.exe:	cmp.c
	$(CC) $(CFLAGS) $(LDFLAGS) cmp.c -o cmp.exe $(LIBS)

clear.exe:	clear.c
	$(CC) $(CFLAGS) $(LDFLAGS) clear.c -o $@ $(LIBS)

date.exe:	date.c
	$(CC) $(CFLAGS) $(LDFLAGS) date.c -o $@ $(LIBS)

df.exe:	df.c
	$(CC) $(CFLAGS) $(LDFLAGS) df.c -o $@ $(LIBS)

exists.exe:	exists.c
	$(CC) $(CFLAGS) $(LDFLAGS) exists.c -o exists.exe $(LIBS)

getenforce:	getenforce.c
	$(CC) $(CFLAGS) $(LDFLAGS) getenforce.c -o $@ $(LIBS) $(SELINUX_LIBS)

getopt.exe:	getopt.c
	$(CC) $(CFLAGS) $(LDFLAGS) getopt.c -o $@ $(LIBS)

hd.exe:	hd.c
	$(CC) $(CFLAGS) $(LDFLAGS) hd.c -o hd.exe $(LIBS)

hostname$(SUFFIX):	hostname.c
	$(CC) $(CFLAGS) $(LDFLAGS) hostname.c -o $@ $(LIBS) $(SOCKET_LIB)

ifconfig:	ifconfig.c
	$(CC) $(CFLAGS) $(LDFLAGS) ifconfig.c -o $@ $(LIBS) $(SOCKET_LIB)

ioctl.exe:	ioctl.c
	$(CC) $(CFLAGS) $(LDFLAGS) ioctl.c -o $@ $(LIBS)

link.exe:	link.c
	$(CC) $(CFLAGS) $(LDFLAGS) link.c -o link.exe $(LIBS)

ls:	ls.c
	$(CC) $(CFLAGS) $(LDFLAGS) ls.c -o $@ $(LIBS) $(SELINUX_LIBS)

ls.exe:	ls.c
	$(CC) -D_USE_LIBPORT=2 $(CFLAGS) $(LDFLAGS) ls.c -o $@ $(LIBS)

md5$(SUFFIX):	md5.c
	$(CC) $(CFLAGS) $(LDFLAGS) md5.c -o $@ $(LIBS) $(CRYPT_LIB)

mkdir.exe:	mkdir.c
	$(CC) $(CFLAGS) $(LDFLAGS) mkdir.c -o mkdir.exe $(LIBS)

mkswap.exe:	mkswap.c
	$(CC) $(CFLAGS) $(LDFLAGS) mkswap.c -o $@ $(LIBS)

modexe.exe:	modexe.c
	$(CC) $(CFLAGS) $(LDFLAGS) modexe.c -o $@ $(LIBS)

modexeb.exe:	modexeb.c
	$(CC) $(CFLAGS) $(LDFLAGS) modexeb.c -o $@ $(LIBS)

more.exe:	more.c
	$(CC) $(CFLAGS) $(LDFLAGS) more.c -o $@ $(LIBS)

mv.exe:	mv.c
	$(CC) $(CFLAGS) $(LDFLAGS) mv.c -o mv.exe $(LIBS)

netstat:	netstat.c
	$(CC) $(CFLAGS) $(LDFLAGS) netstat.c -o $@ $(LIBS) $(SOCKET_LIB)

printenv.exe:	printenv.c
	$(CC) $(CFLAGS) $(LDFLAGS) printenv.c -o printenv.exe $(LIBS)

reboot.exe:	reboot.c
	$(CC) -D_USE_KIOCTL $(CFLAGS) $(LDFLAGS) reboot.c -o $@ $(LIBS)

restorecon:	restorecon.c
	$(CC) $(CFLAGS) $(LDFLAGS) restorecon.c -o restorecon $(LIBS) $(SELINUX_LIBS)

rm.exe:	rm.c
	$(CC) $(CFLAGS) $(LDFLAGS) rm.c -o rm.exe $(LIBS)

rmdir.exe:	rmdir.c
	$(CC) $(CFLAGS) $(LDFLAGS) rmdir.c -o rmdir.exe $(LIBS)

runcon:	runcon.c
	$(CC) $(CFLAGS) $(LDFLAGS) runcon.c -o runcon $(LIBS) $(SELINUX_LIBS)

setconsole:	setconsole.c
	$(CC) $(CFLAGS) $(LDFLAGS) setconsole.c -o setconsole $(LIBS) -lpthread

setenforce:	setenforce.c
	$(CC) $(CFLAGS) $(LDFLAGS) setenforce.c -o setenforce $(LIBS) $(SELINUX_LIBS)

setsebool:	setsebool.c
	$(CC) $(CFLAGS) $(LDFLAGS) setsebool.c -o setsebool $(LIBS) $(SELINUX_LIBS)

sleep.exe:	sleep.c
	$(CC) $(CFLAGS) $(LDFLAGS) sleep.c -o sleep.exe $(LIBS)

tee.exe:	tee.c
	$(CC) $(CFLAGS) $(LDFLAGS) tee.c -o $@ $(LIBS)

touch.exe:	touch.c
	$(CC) $(CFLAGS) $(LDFLAGS) touch.c -o touch.exe $(LIBS)

unlink.exe:	unlink.c
	$(CC) $(CFLAGS) $(LDFLAGS) unlink.c -o unlink.exe $(LIBS)

uptime$(SUFFIX):	uptime.c
	$(CC) $(CFLAGS) $(LDFLAGS) uptime.c -o $@ $(LIBS) $(TIME_LIB) $(MATH_LIB)

which.exe:	which.c
	$(CC) $(CFLAGS) $(LDFLAGS) which.c -o $@ $(LIBS)

libgetopt/libgetopt.a:	libgetopt/getopt.o
#	CC="$(CC)" CFLAGS="$(CFLAGS)" $(MAKE) -C libgetopt libgetopt.a
	$(AR) -rs $@ $^

posix-io-for-windows/libposixio.a:
	$(MAKE) -C posix-io-for-windows
