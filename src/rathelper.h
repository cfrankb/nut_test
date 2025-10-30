#pragma once

#include <sqrat.h>
class CMap;
void VerifyCMapInstance(HSQUIRRELVM v, CMap *expected);
bool IsClassBound(HSQUIRRELVM v, const char *name);
void debugRat(HSQUIRRELVM v);
void DumpStack(HSQUIRRELVM v);
void DumpCMapInstance(HSQUIRRELVM v, int idx = -1);
bool IsRootTable(HSQUIRRELVM v, int idx = 1);