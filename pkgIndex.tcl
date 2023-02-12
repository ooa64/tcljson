package ifneeded tcljson 1.0 \
    "[list load [file join $dir libtcljson.so]];
     [list source [file join $dir jsonencode.tcl]];
     package provide tcljson 1.0"
