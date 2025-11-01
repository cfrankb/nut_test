#include "rathelper.h"
#include <sqrat.h>
#include "map.h"

// === VERIFICATION FUNCTION ===
void VerifyCMapInstance(HSQUIRRELVM v, CMap *expected)
{
    CMap *actual = nullptr;
    if (SQ_FAILED(sq_getinstanceup(v, -1, (SQUserPointer *)&actual, nullptr)))
    {
        LOGE("VERIFY: sq_getinstanceup failed — not a CMap instance");
        return;
    }

    if (actual == expected)
    {
        LOGI("VERIFY: SUCCESS — instance bound to correct g_map (%p)", actual);
    }
    else
    {
        LOGE("VERIFY: FAILED — expected %p, got %p", expected, actual);
    }
}

bool IsClassBound(HSQUIRRELVM v, const char *name)
{
    Sqrat::RootTable rt(v);
    Sqrat::Object obj = rt.GetSlot(name);
    return obj.IsNull() == false && obj.GetType() == OT_CLASS;
}

void debugRat(HSQUIRRELVM v)
{
    LOGI("STACK TOP: %d", sq_gettop(v));
    for (int i = 1; i <= sq_gettop(v); i++)
    {
        LOGI("  [%d] type=%d", i, sq_gettype(v, i));
    }
}

void DumpStack(HSQUIRRELVM v)
{
    SQInteger top = sq_gettop(v);
    LOGI("=== SQUIRREL STACK DUMP (top=%d) ===", top);

    for (SQInteger i = 1; i <= top; ++i)
    {
        SQObjectType type = sq_gettype(v, i);
        const char *typeName = "unknown";

        switch (type)
        {
        case OT_NULL:
            typeName = "null";
            break;
        case OT_INTEGER:
            typeName = "integer";
            break;
        case OT_FLOAT:
            typeName = "float";
            break;
        case OT_STRING:
            typeName = "string";
            break;
        case OT_TABLE:
            typeName = "table";
            break;
        case OT_ARRAY:
            typeName = "array";
            break;
        case OT_USERDATA:
            typeName = "userdata";
            break;
        case OT_CLOSURE:
            typeName = "closure";
            break;
        case OT_NATIVECLOSURE:
            typeName = "nativeclosure";
            break;
        case OT_GENERATOR:
            typeName = "generator";
            break;
        case OT_USERPOINTER:
            typeName = "userpointer";
            break;
        case OT_CLASS:
            typeName = "class";
            break;
        case OT_INSTANCE:
            typeName = "instance";
            break;
        case OT_WEAKREF:
            typeName = "weakref";
            break;
        case OT_BOOL:
            typeName = "bool";
            break;
        }

        LOGI("  [%d] = %s", i, typeName);

        // Print value if possible
        switch (type)
        {
        case OT_INTEGER:
        {
            SQInteger val;
            sq_getinteger(v, i, &val);
            LOGI("       value: %lld", val);
            break;
        }
        case OT_FLOAT:
        {
            SQFloat val;
            sq_getfloat(v, i, &val);
            LOGI("       value: %f", val);
            break;
        }
        case OT_STRING:
        {
            const SQChar *str;
            sq_getstring(v, i, &str);
            LOGI("       value: \"%s\"", str);
            break;
        }
        case OT_BOOL:
        {
            SQBool val;
            sq_getbool(v, i, &val);
            LOGI("       value: %s", val ? "true" : "false");
            break;
        }
        }
    }
    LOGI("=== END STACK DUMP ===");
}

bool IsRootTable(HSQUIRRELVM v, int idx)
{
    sq_pushroottable(v);
    bool equal = (sq_gettype(v, idx) == OT_TABLE) &&
                 (sq_cmp(v) == 0);
    sq_pop(v, 1);
    return equal;
}
