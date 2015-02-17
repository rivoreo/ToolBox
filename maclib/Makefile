CFLAGS = -Wall -arch i386 -arch x86_64 -arch ppc -mmacosx-version-min=10.4

first:	libgetopt.a libcrypto.dylib

libgetopt.a:	getopt.o
	ar -rs $@ $^

libcrypto.dylib:	libcrypto.0.9.7.dylib
	strip $^ -o $@

libcrypto.0.9.7.dylib:	libcrypto.0.9.7.exported.c
	gcc --shared $(CFLAGS) $^ -o $@

libcrypto.0.9.7.exported.c:	libcrypto-0.9.7-export-list
	echo "for i in \$$@; do echo \"void \$$i() {}\"; done" | bash /dev/stdin `cat $^` > $@

clean:
	rm -f libcrypto.0.9.7.exported.c libcrypto.0.9.7.dylib *.o

distclean:	clean
	rm -f *.dylib *.a
