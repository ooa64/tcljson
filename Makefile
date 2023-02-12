TCLROOT?=/usr
TCLVER?=8.6

VERSION = tcljson1.0
TCLFLAGS = -I$(TCLROOT)/include -I$(TCLROOT)/include/tcl
#DEBUGFLAGS = -ggdb
CFLAGS = -fPIC $(TCLFLAGS) $(DEBUGFLAGS)

TCLLIB = -L$(TCLROOT)/lib -ltcl$(TCLVER) -lm -ldl

TARGETS = libtcljson.a libtcljson.so
SCRIPTS = jsonencode.tcl pkgIndex.tcl

OBJs=tcl-json.o jsmn/jsmn.o

all: $(TARGETS)

tcl-json.c: jim-json.c jim-json.sed
	sed -f jim-json.sed < jim-json.c > tcl-json.c

libtcljson.a: $(OBJs)
	rm -f $@
	ar cr $@ $^

libtcljson.so: $(OBJs)
	gcc -shared -o $@ $^ $(TCLLIB)

$(OBJs): Makefile

install: all
	mkdir $(TCLROOT)/lib/$(VERSION)
	cp $(TARGETS) $(SCRIPTS) $(TCLROOT)/lib/$(VERSION)/

test: all
	tclsh json.test

clean:
	rm -f $(TARGETS) *.o core $(VERSION).tgz

tar:
	make clean
	tar -zcf $(VERSION).tgz $(DISTR)
