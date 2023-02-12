# tcljson

Ported from JIM TCL http://jim.tcl.tk/

json::encode
~~~~~~~~~~~~

The Tcl -> JSON encoder is part of the optional 'json' package.

+*json::encode* 'value ?schema?'+::

Encode a Tcl value as JSON according to the schema (defaults to +'str'+). The following schema types are supported:
* 'str' - Tcl string -> JSON string
* 'num' - Tcl value -> bare numeric value or null
* 'bool' - Tcl boolean value -> true, false
* 'obj ?name subschema ...?' - Tcl dict -> JSON object. For each dict key matching 'name', the corresponding 'subschema'
is applied. The special name +'*'+ matches any keys not otherwise matched, otherwise the default +'str'+ is used.
* 'list ?subschema?' - Tcl list -> JSON array. The 'subschema' (default +'str'+) is applied for each element of the list/array.
* 'mixed ?subschema ...?' = Tcl list -> JSON array. Each 'subschema' is applied for the corresponding element of the list/array.
  ::
The following are examples:
----
    . json::encode {1 2 true false null 5.0} list
    [ "1", "2", "true", "false", "null", "5.0" ]
    . json::encode {1 2 true false null 5.0} {list num}
    [ 1, 2, true, false, null, 5.0 ]
    . json::encode {0 1 2 true false 5.0 off} {list bool}
    [ false, true, true, true, false, true, false ]
    . json::encode {a 1 b hello c {3 4}} obj
    { "a":"1", "b":"hello", "c":"3 4" }
    . json::encode {a 1 b hello c {3 4}} {obj a num c {list num}}
    { "a":1, "b":"hello", "c":[ 3, 4 ] }
    . json::encode {true true {abc def}} {mixed str num obj}
    [ "true", true, { "abc":"def" } ]
    . json::encode {a 1 b 3.0 c hello d null} {obj c str * num}
    { "a":1, "b":3.0, "c":"hello", "d":null }
----

json::decode
~~~~~~~~~~~~

The JSON -> Tcl decoder is part of the optional 'json' package.

+*json::decode* ?*-index*? ?*-null* 'string'? ?*-schema*? 'json-string'+::

Decodes the given JSON string (must be array or object) into a Tcl data structure. If '+-index+' is specified,
decodes JSON arrays as dictionaries with numeric keys. This makes it possible to retrieve data from nested
arrays and dictionaries with just '+dict get+'. With the option '+-schema+' returns a list of +'{data schema}'+
where the schema is compatible with `json::encode`. Otherwise just returns the data.
Decoding is as follows (with schema types listed in parentheses):
* object -> dict ('obj')
* array -> list ('mixed' or 'list')
* number -> as-is ('num')
* boolean -> as-is ('bool')
* string -> string ('str')
* null -> supplied null string or the default +'"null"'+ ('num')
 ::
 Note that an object decoded into a dict will return the keys in the same order as the original string.
----
    . json::decode {[1, 2]}
    {1 2}
    . json::decode -schema {[1, 2]}
    {1 2} {list num}
    . json::decode -schema {{"a":1, "b":2}}
    {a 1 b 2} {obj a num b num}
    . json::decode -schema {[1, 2, {a:"b", c:false}, "hello"]}
    {1 2 {a b c false} hello} {mixed num num {obj a str c bool} str}
    . json::decode -index {["foo", "bar"]}
    {0 foo 1 bar}
----
