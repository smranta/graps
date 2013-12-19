
export LD_PRELOAD=

all: libgraps.a grapsctl

libgraps.a : graps.c graps.h
	gcc -g -O2 -Wall -fPIC -c graps.c -o graps.o
	#libtool --mode=compile gcc -g -O2 -Wall -fPIC -c graps.c -o .libs/graps.lo
	#libtool --mode=link gcc  -g -O2 -Wall -fPIC -o .libs/libgraps.la -rpath /usr/local/lib -lglut .libs/graps.lo
	gcc -shared  graps.o  -lglut  -Wl,-soname -Wl,libgraps.so.0 -o libgraps.so.0.0.0
	(rm -f libgraps.so.0 && ln -s libgraps.so.0.0.0 libgraps.so.0)
	(rm -f libgraps.so && ln -s libgraps.so.0.0.0 libgraps.so)
	ar cru libgraps.a  graps.o
	ranlib libgraps.a

grapsctl : grapsctl.c graps.h
	gcc -g -O2 -Wall grapsctl.c -o grapsctl

clean :
	rm -rf grapsctl libgraps* graps.o .libs/* .libs/.libs frames/* output.avi
