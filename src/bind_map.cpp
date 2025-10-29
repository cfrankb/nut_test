#include <squirrel.h>
#include <sqstdstring.h>
#include <sqrat.h>
#include "map.h"

// Helper: Push Pos as table {x, y}
void PushPos(HSQUIRRELVM v, const Pos &pos)
{
    sq_newtable(v);
    sq_pushstring(v, _SC("x"), -1);
    sq_pushinteger(v, pos.x);
    sq_set(v, -3);
    sq_pushstring(v, _SC("y"), -1);
    sq_pushinteger(v, pos.y);
    sq_set(v, -3);
}

// Helper: Read Pos from table
bool TableToPos(HSQUIRRELVM v, SQInteger idx, Pos &pos)
{
    if (sq_gettype(v, idx) != OT_TABLE)
        return false;

    sq_pushstring(v, _SC("x"), -1);
    if (SQ_FAILED(sq_get(v, idx)))
        return false;

    SQInteger x = pos.x;
    SQInteger y = pos.y;

    if (SQ_FAILED(sq_getinteger(v, -1, &x)))
    {
        sq_pop(v, 1);
        return false;
    }
    sq_pop(v, 1);

    sq_pushstring(v, _SC("y"), -1);
    if (SQ_FAILED(sq_get(v, idx)))
        return false;
    if (SQ_FAILED(sq_getinteger(v, -1, &y)))
    {
        sq_pop(v, 1);
        return false;
    }
    sq_pop(v, 1);

    return true;
}

// === CMap Constructor ===
SQInteger CMap_Constructor(HSQUIRRELVM v)
{
    SQInteger len = 0, hei = 0;
    SQUserPointer up = nullptr;
    uint8_t t = 0;

    if (sq_gettop(v) >= 4)
    {
        sq_getinteger(v, 2, &len);
        sq_getinteger(v, 3, &hei);
        SQInteger t_val;
        sq_getinteger(v, 4, &t_val);
        t = static_cast<uint8_t>(t_val);
    }

    CMap *map = new CMap(static_cast<uint16_t>(len), static_cast<uint16_t>(hei), t);
    sq_setinstanceup(v, 1, map);
    sq_setreleasehook(v, 1, +[](SQUserPointer p, SQInteger size) -> SQInteger
                      {
    delete static_cast<CMap*>(p);
    return 0; });

    return 0;
}

// === at(x, y) ===
SQInteger CMap_at(HSQUIRRELVM v)
{
    CMap *map;
    if (SQ_FAILED(sq_getinstanceup(v, 1, (SQUserPointer *)&map, nullptr)))
    {
        return sq_throwerror(v, _SC("CMap instance expected"));
    }

    SQInteger x, y;
    if (SQ_FAILED(sq_getinteger(v, 2, &x)) || SQ_FAILED(sq_getinteger(v, 3, &y)))
    {
        return sq_throwerror(v, _SC("integer x, y expected"));
    }

    if (!map->isValid(x, y))
    {
        return sq_throwerror(v, _SC("coordinates out of bounds"));
    }

    sq_pushinteger(v, map->at(x, y));
    return 1;
}

// === set(x, y, tile) ===
SQInteger CMap_set(HSQUIRRELVM v)
{
    CMap *map;
    if (SQ_FAILED(sq_getinstanceup(v, 1, (SQUserPointer *)&map, nullptr)))
    {
        return sq_throwerror(v, _SC("CMap instance expected"));
    }

    SQInteger x, y, t;
    if (SQ_FAILED(sq_getinteger(v, 2, &x)) ||
        SQ_FAILED(sq_getinteger(v, 3, &y)) ||
        SQ_FAILED(sq_getinteger(v, 4, &t)))
    {
        return sq_throwerror(v, _SC("integer x, y, tile expected"));
    }

    if (!map->isValid(x, y))
    {
        return sq_throwerror(v, _SC("coordinates out of bounds"));
    }

    map->set(x, y, static_cast<uint8_t>(t));
    return 0;
}

// === get(x, y) -> &uint8_t (returns integer by value) ===
SQInteger CMap_get(HSQUIRRELVM v)
{
    CMap *map;
    if (SQ_FAILED(sq_getinstanceup(v, 1, (SQUserPointer *)&map, nullptr)))
    {
        return sq_throwerror(v, _SC("CMap instance expected"));
    }

    SQInteger x, y;
    if (SQ_FAILED(sq_getinteger(v, 2, &x)) || SQ_FAILED(sq_getinteger(v, 3, &y)))
    {
        return sq_throwerror(v, _SC("integer x, y expected"));
    }

    if (!map->isValid(x, y))
    {
        return sq_throwerror(v, _SC("coordinates out of bounds"));
    }

    sq_pushinteger(v, map->get(x, y));
    return 1;
}

// === len() ===
SQInteger CMap_len(HSQUIRRELVM v)
{
    CMap *map;
    if (SQ_FAILED(sq_getinstanceup(v, 1, (SQUserPointer *)&map, nullptr)))
    {
        return sq_throwerror(v, _SC("CMap instance expected"));
    }
    sq_pushinteger(v, map->len());
    return 1;
}

// === hei() ===
SQInteger CMap_hei(HSQUIRRELVM v)
{
    CMap *map;
    if (SQ_FAILED(sq_getinstanceup(v, 1, (SQUserPointer *)&map, nullptr)))
    {
        return sq_throwerror(v, _SC("CMap instance expected"));
    }
    sq_pushinteger(v, map->hei());
    return 1;
}

// === resize(len, hei, tile, fast) ===
SQInteger CMap_resize(HSQUIRRELVM v)
{
    CMap *map;
    if (SQ_FAILED(sq_getinstanceup(v, 1, (SQUserPointer *)&map, nullptr)))
    {
        return sq_throwerror(v, _SC("CMap instance expected"));
    }

    SQInteger len, hei, tile;
    SQBool fast = SQFalse;
    if (SQ_FAILED(sq_getinteger(v, 2, &len)) ||
        SQ_FAILED(sq_getinteger(v, 3, &hei)) ||
        SQ_FAILED(sq_getinteger(v, 4, &tile)))
    {
        return sq_throwerror(v, _SC("invalid arguments"));
    }
    if (sq_gettop(v) >= 5)
        sq_getbool(v, 5, &fast);

    bool result = map->resize(static_cast<uint16_t>(len), static_cast<uint16_t>(hei),
                              static_cast<uint8_t>(tile), fast != 0);
    sq_pushbool(v, result);
    return 1;
}

// === findFirst(tileId) -> Pos or null ===
SQInteger CMap_findFirst(HSQUIRRELVM v)
{
    CMap *map;
    if (SQ_FAILED(sq_getinstanceup(v, 1, (SQUserPointer *)&map, nullptr)))
    {
        return sq_throwerror(v, _SC("CMap instance expected"));
    }

    SQInteger tile;
    if (SQ_FAILED(sq_getinteger(v, 2, &tile)))
    {
        return sq_throwerror(v, _SC("tileId expected"));
    }

    Pos pos = map->findFirst(static_cast<uint8_t>(tile));
    if (pos.x == -1 && pos.y == -1)
    {
        sq_pushnull(v);
    }
    else
    {
        PushPos(v, pos);
    }
    return 1;
}

// === count(tileId) ===
SQInteger CMap_count(HSQUIRRELVM v)
{
    CMap *map;
    if (SQ_FAILED(sq_getinstanceup(v, 1, (SQUserPointer *)&map, nullptr)))
    {
        return sq_throwerror(v, _SC("CMap instance expected"));
    }

    SQInteger tile;
    if (SQ_FAILED(sq_getinteger(v, 2, &tile)))
    {
        return sq_throwerror(v, _SC("tileId expected"));
    }

    sq_pushinteger(v, map->count(static_cast<uint8_t>(tile)));
    return 1;
}

// === fill(tile) ===
SQInteger CMap_fill(HSQUIRRELVM v)
{
    CMap *map;
    if (SQ_FAILED(sq_getinstanceup(v, 1, (SQUserPointer *)&map, nullptr)))
    {
        return sq_throwerror(v, _SC("CMap instance expected"));
    }

    SQInteger tile = 0;
    if (sq_gettop(v) >= 2)
    {
        sq_getinteger(v, 2, &tile);
    }

    map->fill(static_cast<uint8_t>(tile));
    return 0;
}

// === isValid(x, y) ===
SQInteger CMap_isValid(HSQUIRRELVM v)
{
    CMap *map;
    if (SQ_FAILED(sq_getinstanceup(v, 1, (SQUserPointer *)&map, nullptr)))
    {
        return sq_throwerror(v, _SC("CMap instance expected"));
    }

    SQInteger x, y;
    if (SQ_FAILED(sq_getinteger(v, 2, &x)) || SQ_FAILED(sq_getinteger(v, 3, &y)))
    {
        return sq_throwerror(v, _SC("x, y expected"));
    }

    sq_pushbool(v, map->isValid(x, y));
    return 1;
}

// === shift(Direction) ===
SQInteger CMap_shift(HSQUIRRELVM v)
{
    CMap *map;
    if (SQ_FAILED(sq_getinstanceup(v, 1, (SQUserPointer *)&map, nullptr)))
    {
        return sq_throwerror(v, _SC("CMap instance expected"));
    }

    SQInteger dir;
    if (SQ_FAILED(sq_getinteger(v, 2, &dir)))
    {
        return sq_throwerror(v, _SC("Direction expected"));
    }

    if (dir < 0 || dir > CMap::MAX)
    {
        return sq_throwerror(v, _SC("Invalid direction"));
    }

    map->shift(static_cast<CMap::Direction>(dir));
    return 0;
}

// === title() / setTitle(str) ===
SQInteger CMap_title(HSQUIRRELVM v)
{
    CMap *map;
    if (SQ_FAILED(sq_getinstanceup(v, 1, (SQUserPointer *)&map, nullptr)))
    {
        return sq_throwerror(v, _SC("CMap instance expected"));
    }

    if (sq_gettop(v) >= 2)
    {
        const SQChar *str;
        if (SQ_FAILED(sq_getstring(v, 2, &str)))
        {
            return sq_throwerror(v, _SC("string expected"));
        }
        map->setTitle(str);
        return 0;
    }
    else
    {
        sq_pushstring(v, map->title(), -1);
        return 1;
    }
}

// === toKey(x, y) or toKey(pos) ===
SQInteger CMap_toKey(HSQUIRRELVM v)
{
    if (sq_gettop(v) == 3)
    {
        // toKey(x, y)
        SQInteger x, y;
        if (SQ_FAILED(sq_getinteger(v, 2, &x)) || SQ_FAILED(sq_getinteger(v, 3, &y)))
        {
            return sq_throwerror(v, _SC("x, y expected"));
        }
        sq_pushinteger(v, CMap::toKey(static_cast<uint8_t>(x), static_cast<uint8_t>(y)));
    }
    else
    {
        // toKey(pos)
        Pos pos;
        if (!TableToPos(v, 2, pos))
        {
            return sq_throwerror(v, _SC("Pos table {x,y} expected"));
        }
        //  sq_pushinteger(v, CMap::toKey(pos));
    }
    return 1;
}

// === toPos(key) ===
SQInteger CMap_toPos(HSQUIRRELVM v)
{
    SQInteger key;
    if (SQ_FAILED(sq_getinteger(v, 2, &key)))
    {
        return sq_throwerror(v, _SC("key expected"));
    }
    Pos pos = CMap::toPos(static_cast<uint16_t>(key));
    PushPos(v, pos);
    return 1;
}

// === Register CMap to Squirrel ===
void Register_CMap(HSQUIRRELVM v)
{
    // Register Pos as a plain table (no class needed)
    sq_pushroottable(v);

    // === Direction Enum ===
    sq_pushstring(v, _SC("Direction"), -1);
    sq_newtable(v);

    sq_pushstring(v, _SC("UP"), -1);
    sq_pushinteger(v, CMap::UP);
    sq_newslot(v, -3, SQFalse);
    sq_pushstring(v, _SC("DOWN"), -1);
    sq_pushinteger(v, CMap::DOWN);
    sq_newslot(v, -3, SQFalse);
    sq_pushstring(v, _SC("LEFT"), -1);
    sq_pushinteger(v, CMap::LEFT);
    sq_newslot(v, -3, SQFalse);
    sq_pushstring(v, _SC("RIGHT"), -1);
    sq_pushinteger(v, CMap::RIGHT);
    sq_newslot(v, -3, SQFalse);
    sq_pushstring(v, _SC("NOT_FOUND"), -1);
    sq_pushinteger(v, CMap::NOT_FOUND);
    sq_newslot(v, -3, SQFalse);

    sq_newslot(v, -3, SQFalse);

    // === CMap Class ===
    sq_pushstring(v, _SC("CMap"), -1);
    sq_newclass(v, SQFalse);

    // Constructor
    sq_pushstring(v, _SC("constructor"), -1);
    sq_newclosure(v, CMap_Constructor, 0);
    sq_setparamscheck(v, SQ_MATCHTYPEMASKSTRING, _SC("tiii|"));
    sq_newslot(v, -3, SQFalse);

// Methods
#define REG_METHOD(name, func)       \
    sq_pushstring(v, _SC(name), -1); \
    sq_newclosure(v, func, 0);       \
    sq_newslot(v, -3, SQFalse);

    REG_METHOD("at", CMap_at);
    REG_METHOD("set", CMap_set);
    REG_METHOD("get", CMap_get);
    REG_METHOD("len", CMap_len);
    REG_METHOD("hei", CMap_hei);
    REG_METHOD("resize", CMap_resize);
    REG_METHOD("findFirst", CMap_findFirst);
    REG_METHOD("count", CMap_count);
    REG_METHOD("fill", CMap_fill);
    REG_METHOD("isValid", CMap_isValid);
    REG_METHOD("shift", CMap_shift);
    REG_METHOD("title", CMap_title);

    // Static methods
    sq_pushstring(v, _SC("toKey"), -1);
    sq_newclosure(v, CMap_toKey, 0);
    sq_newslot(v, -3, SQFalse);

    sq_pushstring(v, _SC("toPos"), -1);
    sq_newclosure(v, CMap_toPos, 0);
    sq_newslot(v, -3, SQFalse);

    // Finalize class
    sq_newslot(v, -3, SQFalse);

    sq_pop(v, 1); // pop root
}

// --- In C++ ---
void PushCMapInstance(HSQUIRRELVM v, CMap *map)
{
    // 1. Push the class (CMap) onto the stack
    sq_pushroottable(v);
    sq_pushstring(v, _SC("CMap"), -1);
    if (SQ_FAILED(sq_get(v, -2)))
    {
        sq_pop(v, 1);
        return; // CMap not registered
    }

    // 2. Create a new instance (but don't construct!)
    sq_createinstance(v, -1);
    sq_remove(v, -2); // remove class
    sq_remove(v, -2); // remove root

    // 3. Set the native pointer (no release hook!)
    sq_setinstanceup(v, -1, map);

    // IMPORTANT: DO NOT set releasehook — C++ owns the object!
    // sq_setreleasehook(v, -1, ...);  // ← REMOVE THIS
}