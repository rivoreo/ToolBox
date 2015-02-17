NATIVETOOLSDIR := ../WindowsNT-NativeTools/
include $(NATIVETOOLSDIR)rules.mk

CFLAGS += -Wall -O1 -D_WIN32_WNT_NATIVE -D_NO_SELINUX "-Dfprintf(F,...)=printf(__VA_ARGS__)" -Iinclude
#CFLAGS +=
#RM = $(shell which rm)

TOOLS_OBJS = \
	chmod_u.o \
	cmp_u.o \
	date_u.o \
	df_u.o \
	exists_u.o \
	hd_u.o \
	ln_u.o \
	ls_u.o \
	md5_u.o \
	mkdir_u.o \
	mkswap_u.o \
	modexe_u.o \
	modexeb_u.o \
	mv_u.o \
	printenv_u.o \
	rmdir_u.o \
	sleep_u.o \
	touch_u.o \
	uptime_u.o

TOOLS = \
	chmod \
	cmp \
	date \
	df \
	exists \
	hd \
	link \
	ln \
	ls \
	md5 \
	mkdir \
	mkswap \
	modexe \
	modexeb \
	mv \
	printenv \
	rmdir \
	sleep \
	touch \
	unlink \
	uptime


ifdef SHARED_OBJECT
first:	libtoolbox.so
endif

unity:	toolbox
separate:	$(TOOLS)

%.c:	%_u.c
	[ -f $@ ] && { /usr/bin/touch -c $@; exit 0; } || $(SHELL) defmain.sh $*

toolbox:	$(NATIVETOOLSDIR)crtn.o toolbox.o $(TOOLS_OBJS)
	$(LD) $(LDFLAGS) $^ -o $@ $(LIBS) -lcrypto

toolbox.dll:	dllcrt.o $(TOOLS_OBJS)
	$(LD) --shared -e _DllMainCRTStartup --subsystem 1 $^ -o $@ $(LIBS) -lcrypto

libtoolbox.so:	dllcrt.o $(TOOLS_OBJS)
	$(LD) --shared -e _DllMainCRTStartup --subsystem 1 $^ -o $@ $(LIBS) -lcrypto

md5:	$(NATIVETOOLSDIR)crtn.o md5.o
	$(LD) $(LDFLAGS) $^ -o $@ $(LIBS) -lcrypto
