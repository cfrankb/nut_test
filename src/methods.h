#pragma once
#include <sqrat.h>

void testPrint(HSQUIRRELVM vm);
SQInteger print_hello(HSQUIRRELVM vm); // @export:printHello
SQInteger greet(HSQUIRRELVM vm);       // @export
SQInteger add_numbers(HSQUIRRELVM vm); // @export:addNumbers
