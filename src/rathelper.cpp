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

void DumpCMapInstanceX(HSQUIRRELVM v, int idx)
{
    LOGI("=== FULL CMAP INSTANCE DUMP (idx=%d) ===", idx);

    if (sq_gettype(v, idx) != OT_INSTANCE)
    {
        LOGE("ERROR: Not an instance! Type: %d", sq_gettype(v, idx));
        return;
    }

    // 1. Instance object address
    HSQOBJECT obj;
    sq_resetobject(&obj);
    sq_getstackobj(v, idx, &obj);
    LOGI("  Instance object: %p", (void *)obj._unVal.pInstance);

    // 2. User pointer (our CMap*)
    CMap *map = nullptr;
    if (SQ_SUCCEEDED(sq_getinstanceup(v, idx, (SQUserPointer *)&map, nullptr)))
    {
        LOGI("  User pointer (CMap*): %p", map);
        //    LOGI("  g_map address: %p", &g_map);
        //    LOGI("  Match: %s", (map == &g_map) ? "YES" : "NO");
    }
    else
    {
        LOGE("  sq_getinstanceup() FAILED");
    }

    // 3. Class (metatable)
    // HSQOBJECT classObj;
    // sq_resetobject(&classObj);
    // if (SQ_SUCCEEDED(sq_getclass(v, idx)))
    {
        //    LOGI("  Class object: %p", (void *)classObj._unVal.pClass);
    }

    // 4. Dump all slots (methods, fields)
    LOGI("  --- SLOTS ---");
    sq_pushnull(v); // iterator
    while (SQ_SUCCEEDED(sq_next(v, idx)))
    {
        const SQChar *key;
        sq_getstring(v, -2, &key);

        SQObjectType valType = sq_gettype(v, -1);
        const char *typeName = "unknown";
        switch (valType)
        {
        case OT_CLOSURE:
            typeName = "closure";
            break;
        case OT_NATIVECLOSURE:
            typeName = "nativeclosure";
            break;
        case OT_STRING:
            typeName = "string";
            break;
        case OT_INTEGER:
            typeName = "integer";
            break;
        case OT_FLOAT:
            typeName = "float";
            break;
        case OT_BOOL:
            typeName = "bool";
            break;
        default:
            break;
        }

        LOGI("    [%s] = %s", key, typeName);

        // Print value if simple
        if (valType == OT_STRING)
        {
            const SQChar *str;
            sq_getstring(v, -1, &str);
            LOGI("        \"%s\"", str);
        }
        else if (valType == OT_INTEGER)
        {
            SQInteger i;
            sq_getinteger(v, -1, &i);
            LOGI("        %lld", i);
        }

        sq_pop(v, 2); // pop key + value
    }
    sq_pop(v, 1); // pop null iterator

    LOGI("=== END DUMP ===");
}

void DumpCMapClass(HSQUIRRELVM v)
{
    LOGI("=== CMAP CLASS (METATABLE) DUMP ===");

    // Get class from instance
    HSQOBJECT classObj;
    sq_resetobject(&classObj);
    // if (SQ_FAILED(sq_getclass(v, -1, &classObj)))
    {
        //   LOGE("Failed to get class");
        //   return;
    }

    sq_pushobject(v, classObj);

    sq_pushnull(v);
    while (SQ_SUCCEEDED(sq_next(v, -2)))
    {
        const SQChar *key;
        sq_getstring(v, -2, &key);
        LOGI("  [%s] = method", key);
        sq_pop(v, 2);
    }
    sq_pop(v, 2); // pop null + class
}

void DumpCMapInstance(HSQUIRRELVM v, int idx)
{
    LOGI("=== FULL CMAP INSTANCE DUMP (idx=%d) ===", idx);

    // ... user pointer, etc.

    LOGI("  --- INSTANCE SLOTS (usually empty) ---");
    sq_pushnull(v);
    int count = 0;
    while (SQ_SUCCEEDED(sq_next(v, idx)))
    {
        const SQChar *key;
        sq_getstring(v, -2, &key);
        LOGI("    [%s] = (instance field)", key);
        sq_pop(v, 2);
        count++;
    }
    sq_pop(v, 1);
    if (count == 0)
        LOGI("    (no instance fields)");

    // DUMP CLASS METHODS
    DumpCMapClass(v);

    LOGI("=== END DUMP ===");
}

void PushCMapInstance2x(HSQUIRRELVM v, CMap *map)
{
    sq_pushstring(v, "CMap", -1);
    if (SQ_FAILED(sq_get(v, -2)))
    {
        sq_throwerror(v, "CMap not bound");
        return;
    }

    if (SQ_FAILED(sq_createinstance(v, -1)))
    {
        sq_throwerror(v, "Failed to create instance");
        return;
    }

    sq_setinstanceup(v, -1, map);
    sq_remove(v, -2);
}

bool IsRootTable(HSQUIRRELVM v, int idx)
{
    sq_pushroottable(v);
    bool equal = (sq_gettype(v, idx) == OT_TABLE) &&
                 (sq_cmp(v) == 0);
    sq_pop(v, 1);
    return equal;
}
