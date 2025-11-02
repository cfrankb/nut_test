#include <sstream>
#include <iostream>

#include "logger.h"
#include "methods.h"

void testPrint(HSQUIRRELVM vm)
{
    Sqrat::Function printFunc = Sqrat::RootTable(vm).GetFunction("print");
    if (printFunc.IsNull())
        std::cerr << "print() is not bound!" << std::endl;
    else
        printFunc.Release();

    sq_pushroottable(vm);           // stack: [root]
    sq_pushstring(vm, "print", -1); // stack: [root, "print"]
    if (SQ_SUCCEEDED(sq_get(vm, -2)))
    { // stack: [root, print_fn]
        LOGI("print() is bound in root table");
        sq_pop(vm, 2); // clean up: pop print_fn and root
    }
    else
    {
        LOGE("print() is NOT bound in root table");
        sq_pop(vm, 1); // clean up: pop root
    }
}

// Native function to expose to Squirrel
SQInteger print_hello(HSQUIRRELVM vm)
{
    printf("Hello from C++!\n");
    return 0; // number of return values
}

SQInteger add_numbers(HSQUIRRELVM vm)
{
    SQInteger a, b;

    if (sq_gettype(vm, 2) != OT_INTEGER)
    {
        return sq_throwerror(vm, "Expected integer");
    }

    if (sq_gettype(vm, 2) != OT_INTEGER)
    {
        return sq_throwerror(vm, "Expected integer");
    }

    if (SQ_FAILED(sq_getinteger(vm, 2, &a)) || SQ_FAILED(sq_getinteger(vm, 3, &b)))
    {
        return sq_throwerror(vm, "Expected two integers");
    }
    sq_pushinteger(vm, a + b);
    return 1;
}

SQInteger greet(HSQUIRRELVM vm)
{
    const SQChar *name;
    SQInteger times;

    // Argument 1: string (name)
    if (SQ_FAILED(sq_getstring(vm, 2, &name)))
    {
        return sq_throwerror(vm, "Expected string as first argument");
    }

    // Argument 2: integer (times)
    if (SQ_FAILED(sq_getinteger(vm, 3, &times)))
    {
        return sq_throwerror(vm, "Expected integer as second argument");
    }

    for (SQInteger i = 0; i < times; ++i)
    {
        printf("Hello %s!\n", name);
    }

    return 0; // no return values
}
