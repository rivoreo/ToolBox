NATIVETOOLSDIR := ../WindowsNT-NativeTools/
include $(NATIVETOOLSDIR)rules.mk

CFLAGS += -Wall -O1 "-Dfprintf(F,...)=printf(__VA_ARGS__)"
#CFLAGS +=
RM = $(shell which rm)

TOOLS = \
	chmod \
	cmp \
	date \
	exists \
	hd \
	link \
	ln \
	ls \
	md5 \
	mkdir \
	mv \
	printenv \
	rmdir \
	sleep \
	touch \
	unlink \
	uptime

first:	$(TOOLS)

%.c:	%_u.c
	[ -f $@ ] && { /usr/bin/touch -c $@; exit 0; } || $(SHELL) defmain.sh $*

md5:	$(NATIVETOOLSDIR)crtn.o md5.o
	$(LD) $(LDFLAGS) $^ -o $@ $(LIBS) -lcrypto
