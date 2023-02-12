/*
 * Copyright (c) 2015 - 2016 Svyatoslav Mishyn <juef@openmailbox.org>
 * Copyright (c) 2019 Steve Bennett <steveb@workware.net.au>
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all
 * copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "tcl-json.h"

#include "jsmn/jsmn.h"

/* These are all the schema types we support */
typedef enum {
	JSON_BOOL,
	JSON_OBJ,
	JSON_LIST,
	JSON_MIXED,
	JSON_STR,
	JSON_NUM,
	JSON_MAX_TYPE,
} json_schema_t;

struct json_state {
	Tcl_Obj *nullObj;
	const char *json;
	jsmntok_t *tok;
	int need_subst;
	/* The following are used for -schema */
	int enable_schema;
	int enable_index;
	Tcl_Obj *schemaObj;
	Tcl_Obj *schemaTypeObj[JSON_MAX_TYPE];
};

static void json_decode_dump_value(Tcl_Interp *interp, struct json_state *state, Tcl_Obj *list);

/**
 * Start a new subschema. Returns the previous schemaObj.
 * Does nothing and returns NULL if -schema is not enabled.
 */
static Tcl_Obj *json_decode_schema_push(Tcl_Interp *interp, struct json_state *state)
{
	Tcl_Obj *prevSchemaObj = NULL;
	if (state->enable_schema) {
		prevSchemaObj = state->schemaObj;
		state->schemaObj = Tcl_NewListObjJ(interp, NULL, 0);
		Tcl_IncrRefCount(state->schemaObj);
	}
	return prevSchemaObj;
}

/**
 * Combines the current schema with the previous schema, prevSchemaObj
 * returned by json_decode_schema_push().
 * Does nothing if -schema is not enabled.
 */
static void json_decode_schema_pop(Tcl_Interp *interp, struct json_state *state, Tcl_Obj *prevSchemaObj)
{
	if (state->enable_schema) {
		Tcl_ListAppendElementJ(interp, prevSchemaObj, state->schemaObj);
		Tcl_DecrRefCountJ(interp, state->schemaObj);
		state->schemaObj = prevSchemaObj;
	}
}

/**
 * Appends the schema type to state->schemaObj based on 'type'
 */
static void json_decode_add_schema_type(Tcl_Interp *interp, struct json_state *state, json_schema_t type)
{
	static const char * const schema_names[] = {
		"bool",
		"obj",
		"list",
		"mixed",
		"str",
		"num",
	};
	assert(type >= 0 && type < JSON_MAX_TYPE);
	/* Share multiple instances of the same type */
	if (state->schemaTypeObj[type] == NULL) {
		state->schemaTypeObj[type] = Tcl_NewStringObjJ(interp, schema_names[type], -1);
	}
	Tcl_ListAppendElementJ(interp, state->schemaObj, state->schemaTypeObj[type]);
}

/**
 * Returns the schema type for the given token.
 * There is a one-to-one correspondence except for JSMN_PRIMITIVE
 * which will return JSON_BOOL for true, false and JSON_NUM otherise.
 */
static json_schema_t json_decode_get_type(const jsmntok_t *tok, const char *json)
{
	switch (tok->type) {
		case JSMN_PRIMITIVE:
			assert(json);
			if (json[tok->start] == 't' || json[tok->start] == 'f') {
				return JSON_BOOL;
			}
			return JSON_NUM;
		case JSMN_OBJECT:
			return JSON_OBJ;
		case JSMN_ARRAY:
			/* Return mixed by default - need other checks to select list instead */
			return JSON_MIXED;
		case JSMN_STRING:
		default:
			return JSON_STR;
	}
}

/**
 * Returns the current object (state->tok) as a Tcl list.
 *
 * state->tok is incremented to just past the object that was dumped.
 */
static Tcl_Obj *
json_decode_dump_container(Tcl_Interp *interp, struct json_state *state)
{
	int i;
	Tcl_Obj *list = Tcl_NewListObjJ(interp, NULL, 0);
	int size = state->tok->size;
	int type = state->tok->type;
	json_schema_t container_type = JSON_OBJ; /* JSON_LIST, JSON_MIXED or JSON_OBJ */

	if (state->schemaObj) {
		/* Don't strictly need to initialise this, but some compilers can't figure out it is always
		 * assigned a value below.
		 */
		json_schema_t list_type = JSON_STR;
		/* Figure out the type to use for the container */
		if (type == JSMN_ARRAY) {
			/* If every element of the array is of the same primitive schema type (str, bool or num),
			 * we can use "list", otherwise need to use "mixed"
			 */
			container_type = JSON_LIST;
			if (size) {
				list_type = json_decode_get_type(&state->tok[1], state->json);

				if (list_type == JSON_BOOL || list_type == JSON_STR || list_type == JSON_NUM) {
					for (i = 2; i <= size; i++) {
						if (json_decode_get_type(state->tok + i, state->json) != list_type) {
							/* Can't use list */
							container_type = JSON_MIXED;
							break;
						}
					}
				}
				else {
					container_type = JSON_MIXED;
				}
			}
		}
		json_decode_add_schema_type(interp, state, container_type);
		if (container_type == JSON_LIST && size) {
			json_decode_add_schema_type(interp, state, list_type);
		}
	}

	state->tok++;

	for (i = 0; i < size; i++) {
		if (type == JSMN_OBJECT) {
			/* Dump the object key */
			if (state->enable_schema) {
				const char *p = state->json + state->tok->start;
				int len = state->tok->end - state->tok->start;
				Tcl_ListAppendElementJ(interp, state->schemaObj, Tcl_NewStringObjJ(interp, p, len));
			}
			json_decode_dump_value(interp, state, list);
		}

		if (state->enable_index && type == JSMN_ARRAY) {
			Tcl_ListAppendElementJ(interp, list, Tcl_NewIntObjJ(interp, i));
		}

		if (state->schemaObj && container_type != JSON_LIST) {
			if (state->tok->type == JSMN_STRING || state->tok->type == JSMN_PRIMITIVE) {
				json_decode_add_schema_type(interp, state, json_decode_get_type(state->tok, state->json));
			}
		}

		/* Dump the array or object value */
		json_decode_dump_value(interp, state, list);
	}

	return list;
}

/**
 * Appends the value at state->tok to 'list' and increments state->tok to just
 * past that token.
 *
 * Also appends to the schema if state->enable_schema is set.
 */
static void
json_decode_dump_value(Tcl_Interp *interp, struct json_state *state, Tcl_Obj *list)
{
	const jsmntok_t *t = state->tok;

	if (t->type == JSMN_STRING || t->type == JSMN_PRIMITIVE) {
		Tcl_Obj	*elem;
		int len = t->end - t->start;
		const char *p = state->json + t->start;
		if (t->type == JSMN_STRING) {
			/* Do we need to process backslash escapes? */
			if (state->need_subst == 0 && memchr(p, '\\', len) != NULL) {
				state->need_subst = 1;
			}
			elem = Tcl_NewStringObjJ(interp, p, len);
		} else if (p[0] == 'n') {	/* null */
			elem = state->nullObj;
		} else if (p[0] == 'I') {
			elem = Tcl_NewStringObjJ(interp, "Inf", -1);
		} else if (p[0] == '-' && p[1] == 'I') {
			elem = Tcl_NewStringObjJ(interp, "-Inf", -1);
		} else {		/* number, true or false */
			elem = Tcl_NewStringObjJ(interp, p, len);
		}

		Tcl_ListAppendElementJ(interp, list, elem);
		state->tok++;
	}
	else {
		Tcl_Obj *prevSchemaObj = json_decode_schema_push(interp, state);
		Tcl_Obj *newList = json_decode_dump_container(interp, state);
		Tcl_ListAppendElementJ(interp, list, newList);
		json_decode_schema_pop(interp, state, prevSchemaObj);
	}
}

/* Parses the options ?-null string? ?-schema? *state.
 * Any options not present are not set.
 *
 * Returns TCL_OK or TCL_ERROR and sets an error result.
 */
static int parse_json_decode_options(Tcl_Interp *interp, int argc, Tcl_Obj *const argv[], struct json_state *state)
{
	static const char * const options[] = { "-index", "-null", "-schema", NULL };
	enum { OPT_INDEX, OPT_NULL, OPT_SCHEMA, };
	int i;

	for (i = 1; i < argc - 1; i++) {
		int option;
		if (Tcl_GetEnumJ(interp, argv[i], options, &option, NULL, TCL_ERRORMSG | TCL_ENUM_ABBREV) != TCL_OK) {
			return TCL_ERROR;
		}
		switch (option) {
			case OPT_INDEX:
				state->enable_index = 1;
				break;

			case OPT_NULL:
				i++;
				Tcl_IncrRefCount(argv[i]);
				Tcl_DecrRefCountJ(interp, state->nullObj);
				state->nullObj = argv[i];
				break;

			case OPT_SCHEMA:
				state->enable_schema = 1;
				break;
		}
	}

	if (i != argc - 1) {
		Tcl_WrongNumArgs(interp, 1, argv,
			"?-index? ?-null nullvalue? ?-schema? json");
		return TCL_ERROR;
	}

	return TCL_OK;
}

/**
 * Use jsmn to tokenise the JSON string 'json' of length 'len'
 *
 * Returns an allocated array of tokens or NULL on error (and sets an error result)
 */
static jsmntok_t *
json_decode_tokenize(Tcl_Interp *interp, const char *json, size_t len)
{
	jsmntok_t	*t;
	jsmn_parser	 parser;
	int n;

	/* Parse once just to find the number of tokens */
	jsmn_init(&parser);
	n = jsmn_parse(&parser, json, len, NULL, 0);

error:
	switch (n) {
		case JSMN_ERROR_INVAL:
			Tcl_SetResultStringJ(interp, "invalid JSON string", -1);
			return NULL;

		case JSMN_ERROR_PART:
			Tcl_SetResultStringJ(interp, "truncated JSON string", -1);
			return NULL;

		case 0:
			Tcl_SetResultStringJ(interp, "root element must be an object or an array", -1);
			return NULL;

		default:
			break;
	}

	if (n < 0) {
		return NULL;
	}

	t = Tcl_Alloc(n * sizeof(*t));

	jsmn_init(&parser);
	n = jsmn_parse(&parser, json, len, t, n);
	if (t->type != JSMN_OBJECT && t->type != JSMN_ARRAY) {
		n = 0;
	}
	if (n <= 0) {
		Tcl_Free(t);
		goto error;
	}

	return t;
}

/**
 * json::decode returns the decoded data structure.
 *
 * If -schema is specified, returns a list of {data schema}
 */
static int
json_decode(ClientData clientData,Tcl_Interp *interp, int argc, Tcl_Obj *const argv[])
{
	Tcl_Obj *list;
	jsmntok_t *tokens;
	int len;
	int ret = TCL_ERROR;
	struct json_state state;

	memset(&state, 0, sizeof(state));

	state.nullObj = Tcl_NewStringObjJ(interp, "null", -1);
	Tcl_IncrRefCount(state.nullObj);

	if (parse_json_decode_options(interp, argc, argv, &state) != TCL_OK) {
		goto done;
	}

	state.json = Tcl_GetStringJ(argv[argc - 1], &len);

	if (!len) {
		Tcl_SetResultStringJ(interp, "empty JSON string", -1);
		goto done;
	}
	if ((tokens = json_decode_tokenize(interp, state.json, len)) == NULL) {
		goto done;
	}
	state.tok = tokens;
	json_decode_schema_push(interp, &state);

	list = json_decode_dump_container(interp, &state);
	Tcl_Free(tokens);
	ret = TCL_OK;

	/* Make sure the refcount doesn't go to 0 during Tcl_SubstObjJ() */
	Tcl_IncrRefCount(list);

	if (state.need_subst) {
		/* Subsitute backslashes in the returned dictionary.
		 * Need to be careful of refcounts.
		 * Note that Tcl_SubstObjJ() supports a few more escapes than
		 * JSON requires, but should give the same result for all legal escapes.
		 */
		Tcl_Obj *newList;
		Tcl_SubstObjJ(interp, list, &newList, TCL_SUBST_BACKSLASHES);
		Tcl_IncrRefCount(newList);
		Tcl_DecrRefCountJ(interp, list);
		list = newList;
	}

	if (state.schemaObj) {
		Tcl_Obj *resultObj = Tcl_NewListObjJ(interp, NULL, 0);
		Tcl_ListAppendElementJ(interp, resultObj, list);
		Tcl_ListAppendElementJ(interp, resultObj, state.schemaObj);
		Tcl_SetResultJ(interp, resultObj);
		Tcl_DecrRefCountJ(interp, state.schemaObj);
	}
	else {
		Tcl_SetResultJ(interp, list);
	}
	Tcl_DecrRefCountJ(interp, list);

done:
	Tcl_DecrRefCountJ(interp, state.nullObj);

	return ret;
}

int
Tcljson_Init(Tcl_Interp *interp)
{
	Tcl_PackageProvideCheckJ(interp, "json");
	Tcl_CreateCommandJ(interp, "json::decode", json_decode, NULL, NULL);
	/* Load the Tcl implementation of the json encoder if possible */
	Tcl_PackageRequireJ(interp, "jsonencode", 0);
	return TCL_OK;
}
