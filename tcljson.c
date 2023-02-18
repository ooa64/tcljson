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
#define Tcl_PackageRequireJ(a,b,c) Tcl_EvalFile(a,b)
#define Tcl_CreateCommandJ(a,b,c,d,e) Tcl_CreateObjCommand((a),(b),(c),(d),(e))

#include "tcl-json.c"

#undef TCL_STORAGE_CLASS
#define TCL_STORAGE_CLASS DLLEXPORT

DLLEXPORT int
Tcljson_Init(Tcl_Interp* interp)
{
    Tcl_CmdInfo info;

    if (Tcl_InitStubs(interp, "8.1", 0) == NULL) {
        return TCL_ERROR;
    }

    if (Tcl_PkgProvideEx(interp, PACKAGE_NAME, PACKAGE_VERSION, NULL) != TCL_OK) {
        return TCL_ERROR;
    }
    Tcl_CreateObjCommand(interp, "json::decode", (Tcl_ObjCmdProc *)json_decode, NULL, NULL);
    return TCL_OK;
}

