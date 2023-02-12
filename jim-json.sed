s/#include <jim.h>/#include "tcl-json.h"/g
s/Jim_NewListObj/Tcl_NewListObjJ/g
s/Jim_ListAppendElement/Tcl_ListAppendElementJ/g
s/Jim_DecrRefCount/Tcl_DecrRefCountJ/g
s/Jim_NewStringObj/Tcl_NewStringObjJ/g
s/Jim_NewIntObj/Tcl_NewIntObjJ/g
s/Jim_GetEnum/Tcl_GetEnumJ/g
s/Jim_SetResultString/Tcl_SetResultStringJ/g
s/Jim_GetString/Tcl_GetStringJ/g
s/Jim_SubstObj/Tcl_SubstObjJ/g
s/Jim_SetResult/Tcl_SetResultJ/g
s/Jim_PackageProvideCheck/Tcl_PackageProvideCheckJ/g
s/Jim_PackageRequire/Tcl_PackageRequireJ/g
s/Jim_CreateCommand/Tcl_CreateCommandJ/g
s/Jim_jsonInit/Tcljson_Init/g
s/json_decode\s*(\s*Jim_Interp/json_decode(ClientData clientData,Tcl_Interp/
s/JIM_SUBST_FLAG\s*|\s*JIM_SUBST_NOCMD\s*|\s*JIM_SUBST_NOVAR/TCL_SUBST_BACKSLASHES/g
s/JIM_ERR/TCL_ERROR/g
s/Jim/Tcl/g
s/JIM/TCL/g
