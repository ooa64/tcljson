#include <tcl.h>

#define Tcl_NewListObjJ(a,b,c) Tcl_NewListObj((c),(b))
#define Tcl_ListAppendElementJ(a,b,c) Tcl_ListObjAppendElement((a),(b),(c))
#define Tcl_DecrRefCountJ(a,b) Tcl_DecrRefCount((b))
#define Tcl_NewStringObjJ(a,b,c) Tcl_NewStringObj((b),(c))
#define Tcl_NewIntObjJ(a,b) Tcl_NewIntObj((b))
#define Tcl_GetEnumJ(a,b,c,d,e,f) Tcl_GetIndexFromObj((a),(b),(c),(e),0,(d))
#define Tcl_SetResultStringJ(a,b,c) Tcl_SetResult((a),(b),NULL)
#define Tcl_GetStringJ(a,b) Tcl_GetStringFromObj((a),(b))
#define Tcl_SubstObjJ(a,b,c,d) (*(c)=Tcl_SubstObj((a),(b),(d)))
#define Tcl_SetResultJ(a,b) Tcl_SetObjResult((a),(b))
#define Tcl_PackageProvideCheckJ(a,b) Tcl_PkgProvide((a),(b),"1.0")
#define Tcl_PackageRequireJ(a,b,c) Tcl_PkgRequire(a,b,"1.0",0)
#define Tcl_CreateCommandJ(a,b,c,d,e) Tcl_CreateObjCommand((a),(b),(c),(d),(e))
/*
#define json_decodeJ(a,b,c) json_decodeJ(ClientData clientData,a,b,c)

Tcl_GetIndexFromObj(interp, objPtr, tablePtr, msg, flags, indexPtr)
JIM_EXPORT int Jim_GetEnum (Jim_Interp *interp, Jim_Obj *objPtr,
        const char * const *tablePtr, int *indexPtr, const char *name, int flags);

Tcl_SubstObj(interp, list, &newList, TCL_SUBST_FLAG | TCL_SUBST_NOCMD | TCL_SUBST_NOVAR);
Tcl_SubstObj(interp, objPtr, flags)

Tcl_SetResult(interp, resultObj)
Tcl_SetObjResult(interp, objPtr)

Tcl_PackageProvideCheck(interp, "json");
Tcl_PkgProvide(interp, name, version)
*/

#undef TCL_STORAGE_CLASS
#define TCL_STORAGE_CLASS DLLEXPORT

EXTERN int Tcljson_Init( Tcl_Interp *ip );
