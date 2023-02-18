TCLROOT?=/usr
TCLVER?=8.6

PACKAGE_NAME="tcljson"
PACKAGE_VERSION="1.0"
TCLFLAGS = -I$(TCLROOT)/include -I$(TCLROOT)/include/tcl
#DEBUGFLAGS = -ggdb
CFLAGS = -fPIC $(TCLFLAGS) $(DEBUGFLAGS) -DPACKAGE_NAME=\"tcljson\" -DPACKAGE_VERSION=\"1.0\"

TCLLIB = -L$(TCLROOT)/lib -ltcl$(TCLVER) -lm -ldl

TARGETS = libtcljson.a libtcljson.so
SCRIPTS = jsonencode.tcl pkgIndex.tcl
OBJS=tcljson.o jsmn/jsmn.o

all: $(TARGETS)

libtcljson.a: $(OBJS)
	rm -f $@
	ar cr $@ $^

libtcljson.so: $(OBJS)
	gcc -shared -o $@ $^ $(TCLLIB)

$(OBJS): Makefile tcl-json.c

tcl-json.c: jim-json.c jim-json.sed
	sed -f jim-json.sed < jim-json.c > tcl-json.c

install: all
	-mkdir $(TCLROOT)/lib/$(PACKAGE_NAME)$(PACKAGE_VERSION)
	cp $(TARGETS) $(SCRIPTS) $(TCLROOT)/lib/$(PACKAGE_NAME)$(PACKAGE_VERSION)/

test: all
	tclsh json.test

clean:
	rm -f $(TARGETS) *.o tcl-json.c core
