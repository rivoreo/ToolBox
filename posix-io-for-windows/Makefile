# 工具箱在 Windows 上的重要辅助库的 Makefile

ifeq ($(AR),ar)
AR = i586-mingw32msvc-ar
endif

ifeq ($(CC),cc)
CC = i586-mingw32msvc-gcc
endif

CFLAGS = -O1 -Wall

#OBJS = open.o read.o write.o close.o
OBJS = \
	close.o \
	fileno.o \
	lseek.o \
	open.o \
	read.o \
	write.o

libposixio.a:	$(OBJS)
	$(AR) -r $@ $^

clean:
	rm -f *.o

distclean:	clean
	rm -f *.a
