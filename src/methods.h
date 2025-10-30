#pragma once
#include <sqrat.h>

void testPrint(HSQUIRRELVM vm);
// @export:printHello
SQInteger print_hello(HSQUIRRELVM vm);
// @export
SQInteger greet(HSQUIRRELVM vm);
// @export:addNumbers
SQInteger add_numbers(HSQUIRRELVM vm);
