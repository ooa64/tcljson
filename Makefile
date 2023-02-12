TCLROOT?=/usr
TCLVER?=8.6

VERSION = tcljson-1.0
TCLFLAGS = -I$(TCLROOT)/include -I$(TCLROOT)/include/tcl
#DEBUGFLAGS = -ggdb
CFLAGS = -fPIC $(TCLFLAGS) $(DEBUGFLAGS)

TCLLIB = -L$(TCLROOT)/lib -ltcl$(TCLVER) -lm -ldl

TARGETS = libtcljson.a libtcljson.so

OBJs=tcl-json.o jsmn/jsmn.o

all: $(TARGETS)

libtcljson.a: $(OBJs)
	rm -f $@
	ar cr $@ $^

libtcljson.so: $(OBJs)
	gcc -shared -o $@ $^ $(TCLLIB)

$(OBJs): Makefile

test:
	tclsh json.test

clean:
	rm -f $(TARGETS) *.o core $(VERSION).tgz

tar:
	make clean
	tar -zcf $(VERSION).tgz $(DISTR)
