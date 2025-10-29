#pragma once
#include <sqrat.h>

class CMap;

void PushCMapInstance(HSQUIRRELVM v, CMap *map);
void Register_CMap(HSQUIRRELVM v);
