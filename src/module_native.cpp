#include "module_natve.h"
#include "logger.h"
#include "sqrat.h"

SQInteger native_parse_args(HSQUIRRELVM vm)
{
    SQInteger a, b;

    SQInteger nargs = sq_gettop(vm); // includes 'this' if method, or just args if global
    LOGI("Number of arguments: %d\n", nargs);

    for (SQInteger i = 1; i <= nargs; ++i)
    {
        if (sq_gettype(vm, i) == OT_INTEGER)
        {
            SQInteger val;
            if (SQ_SUCCEEDED(sq_getinteger(vm, i, &val)))
            {
                LOGI("arg %d is int: %d\n", i, val);
            }
            else
            {
                sq_throwerror(vm, "failed to retrieve int");
            }
        }
        else if (sq_gettype(vm, i) == OT_STRING)
        {
            const SQChar *s;
            if (SQ_SUCCEEDED(sq_getstring(vm, i, &s)))
            {
                LOGI("arg %d is str: %s\n", i, s);
            }
            else
            {
                sq_throwerror(vm, "failed to retrieve str");
            }
        }
        else if (sq_gettype(vm, i) == OT_BOOL)
        {
            SQBool boolv;
            if (SQ_SUCCEEDED(sq_getbool(vm, i, &boolv)))
            {
                LOGI("arg %d is bool: %s\n", i, boolv ? "true" : "false");
            }
            else
            {
                sq_throwerror(vm, "failed to retrieve bool");
            }
        }
        else if (i == 1)
        {
            LOGI("arg %d is the this pointer for the module", i);
            HSQOBJECT thisObj;
            sq_getstackobj(vm, 1, &thisObj);
            sq_pushobject(vm, thisObj);
            sq_tostring(vm, -1);
            const SQChar *str;
            sq_getstring(vm, -1, &str);
            LOGI("this = %s", str);
            sq_pop(vm, 1);
        }
        else
        {
            LOGI("arg %d type %d: unsuppoted\n", i, sq_gettype(vm, i));
        }
    }

    sq_pushinteger(vm, nargs);
    return 1;
}

SQInteger native_onLoad(HSQUIRRELVM vm)
{
    LOGI("native::onLoad");
    return 0;
}

SQInteger native_onUnload(HSQUIRRELVM vm)
{
    LOGI("native::onUnload");
    return 0;
}

SQInteger native_entryPoint(HSQUIRRELVM vm)
{
    Sqrat::Table exports(vm);

    auto pushFunc = [vm, &exports](const char *name, const SQFUNCTION &func)
    {
        // Push exports table
        sq_pushobject(vm, exports.GetObject());
        // Manually bind raw VM function
        sq_pushstring(vm, name, -1);
        sq_newclosure(vm, func, 0);
        // Stack: ... table key value
        sq_newslot(vm, -3, SQFalse);
        // Pop exports table (still on stack)
        sq_pop(vm, 1);
    };

    LOGI("native entryPoint");

    pushFunc("parse_args", &native_parse_args);
    pushFunc("onLoad", &native_onLoad);
    pushFunc("onUnload", &native_onUnload);

    // Push exports table onto the VM stack
    sq_pushobject(vm, exports.GetObject());

    return 1; // returning one object (the exports table)
}