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
ifeq ($(OS_NAME),Darwin)
DARWIN = 1
endif
ifeq ($(OS_NAME),GNU)
GNU = 1
endif
endif

CFLAGS += -Iinclude -O1 -Wall

ifdef DARWIN
LIB_NAME = libtoolbox.dylib
else
LIB_NAME = libtoolbox.so
endif

ifdef SHARED_OBJECT
CFLAGS += -fPIC
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
ifndef NO_STATIC
UNITY_LDFLAGS = --static
endif
endif		# !NO_STATIC
endif		# SHARED_OBJECT

ALL_TOOLS := \
	cat_u.o \
	chmod_u.o \
	chown_u.o \
	clear_u.o \
	cmp_u.o \
	date_u.o \
	dd_u.o \
	df_u.o \
	du_u.o \
	exists_u.o \
	hd_u.o \
	hostname_u.o \
	id_u.o \
	ifconfig_u.o \
	iftop_u.o \
	ioctl_u.o \
	isptrace1allowed_u.o \
	kill_u.o \
	kill1_u.o \
	ln_u.o \
	ls_u.o \
	lsof_u.o \
	md5_u.o \
	mkdir_u.o \
	mknod_u.o \
	mkswap_u.o \
	modexe_u.o \
	modexeb_u.o \
	mtdread_u.o \
	mv_u.o \
	netstat_u.o \
	nohup_u.o \
	ps_u.o \
	r_u.o \
	readtty_u.o \
	reboot_u.o \
	rm_u.o \
	rmdir_u.o \
	schedtop_u.o \
	sleep_u.o \
	sync_u.o \
	top_u.o \
	touch_u.o \
	uptime_u.o \
	which_u.o

ifdef MINGW
SUFFIX := .exe
CFLAGS += -D_NO_SELINUX
ifeq ($(CC),arm-mingw32ce-gcc)
CFLAGS += --include include/wcedef.h
LIBS += -Lwcelib -lc
TIMELIB = -lmmtimer
endif
ifeq ($(CC),i586-mingw32msvc-gcc)
LIBS += -Lposix-io-for-windows -lposixio
DEPEND = posix-io-for-windows/libposixio.a
export CONSOLE = 1
TIMELIB = -lwinmm
endif
else
EXTRA_TOOLS := \
	chown \
	dd \
	df \
	du \
	id \
	ifconfig \
	iftop \
	isptrace1allowed \
	kill \
	kill1 \
	ln \
	lsof \
	mknod \
	mtdread \
	netstat \
	ps \
	r \
	readtty \
	schedtop \
	sync \
	top

ifdef DARWIN
NO_SELINUX = 1
CFLAGS += -D_NO_UTIMENSAT -fnested-functions
LIBS += -Lmaclib -lgetopt
DEPEND += maclib/libgetopt.a
ifndef SHARED_OBJECT
ALL_TOOLS += printenv_u.o
endif
else
ALL_TOOLS += \
	dmesg_u.o \
	printenv_u.o \
	renice_u.o \
	vmstat_u.o
EXTRA_TOOLS += \
	dmesg \
	renice \
	vmstat
TIMELIB = -lrt
ifdef GNU
NO_SELINUX = 1
# utimensat is not implemented in GNU/Hurd
CFLAGS += "-DPATH_MAX=(512)" -D_NO_UTIMENSAT
else
ALL_TOOLS += \
	getevent_u.o \
	insmod_u.o \
	lsmod_u.o \
	notify_u.o \
	rmmod_u.o \
	rotatefb_u.o \
	route_u.o \
	sendevent_u.o \
	setconsole_u.o \
	setkey_u.o \
	swapoff_u.o \
	swapon_u.o
EXTRA_TOOLS +=  \
	getevent \
	insmod \
	lsmod \
	notify \
	rmmod \
	rotatefb \
	route \
	sendevent \
	setconsole \
	setkey \
	swapoff \
	swapon
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
	mv$(SUFFIX) \
	printenv$(SUFFIX) \
	reboot$(SUFFIX) \
	rm$(SUFFIX) \
	rmdir$(SUFFIX) \
	sleep$(SUFFIX) \
	touch$(SUFFIX) \
	unlink$(SUFFIX) \
	uptime$(SUFFIX) \
	which$(SUFFIX)


TRAN_SRC = \
	cat.c \
	chcon.c \
	chown.c \
	clear.c \
	cmp.c \
	du.c \
	exists.c \
	getenforce.c \
	getevent.c \
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
	mv.c \
	netstat.c \
	nohup.c \
	notify.c \
	ps.c \
	top.c \
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
	$(CC) $(LDFLAGS) $(UNITY_LDFLAGS) $^ -o $@ $(LIBS) $(SELINUX_LIBS) $(TIMELIB) -lcrypto -lpthread

#separate-mingw:

%.c:	%_u.c
	[ -f $@ ] && { $(TOUCH) -c $@; exit 0; } || $(SHELL) defmain.sh $*

lsmod.c:	;

cleanc:
	/bin/rm -f $(TRAN_SRC)

clean:	cleanc
	/bin/rm -f toolbox toolbox.dll $(LIB_NAME) $(BASE_TOOLS) $(EXTRA_TOOLS) *.o *.exe
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

hd.exe:	hd.c
	$(CC) $(CFLAGS) $(LDFLAGS) hd.c -o hd.exe $(LIBS)

hostname.exe:	hostname.c
	$(CC) $(CFLAGS) $(LDFLAGS) hostname.c -o $@ $(LIBS)

ioctl.exe:	ioctl.c
	$(CC) $(CFLAGS) $(LDFLAGS) ioctl.c -o $@ $(LIBS)

link.exe:	link.c
	$(CC) $(CFLAGS) $(LDFLAGS) link.c -o link.exe $(LIBS)

ls:	ls.c
	$(CC) $(CFLAGS) $(LDFLAGS) ls.c -o $@ $(LIBS) $(SELINUX_LIBS)

ls.exe:	ls.c
	$(CC) -D_USE_LIBPORT=2 $(CFLAGS) $(LDFLAGS) ls.c -o $@ $(LIBS)

md5$(SUFFIX):	md5.c
	$(CC) $(CFLAGS) $(LDFLAGS) md5.c -o $@ $(LIBS) -lcrypto

mkdir.exe:	mkdir.c
	$(CC) $(CFLAGS) $(LDFLAGS) mkdir.c -o mkdir.exe $(LIBS)

mkswap.exe:	mkswap.c
	$(CC) $(CFLAGS) $(LDFLAGS) mkswap.c -o $@ $(LIBS)

modexe.exe:	modexe.c
	$(CC) $(CFLAGS) $(LDFLAGS) modexe.c -o $@ $(LIBS)

modexeb.exe:	modexeb.c
	$(CC) $(CFLAGS) $(LDFLAGS) modexeb.c -o $@ $(LIBS)

mv.exe:	mv.c
	$(CC) $(CFLAGS) $(LDFLAGS) mv.c -o mv.exe $(LIBS)

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

touch.exe:	touch.c
	$(CC) $(CFLAGS) $(LDFLAGS) touch.c -o touch.exe $(LIBS)

unlink.exe:	unlink.c
	$(CC) $(CFLAGS) $(LDFLAGS) unlink.c -o unlink.exe $(LIBS)

uptime$(SUFFIX):	uptime.c
	$(CC) $(CFLAGS) $(LDFLAGS) uptime.c -o $@ $(LIBS) $(TIMELIB)

which.exe:	which.c
	$(CC) $(CFLAGS) $(LDFLAGS) which.c -o $@ $(LIBS)

maclib/libgetopt.a:
	$(MAKE) -C maclib libgetopt.a

posix-io-for-windows/libposixio.a:
	$(MAKE) -C posix-io-for-windows
