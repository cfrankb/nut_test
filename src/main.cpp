
#include <squirrel.h>
#include <sqstdio.h>
#include <sqstdaux.h>
#include <sqrat.h>
#include <cstdio> // Required for printf
#include <cstdarg>
#include <sqstdmath.h>
#include <sqstdstring.h>
#include <sqstdsystem.h>
#include "entity.h"
#include "logger.h"
#include "treerat.h"
#include "bind.h"
#include "methods.h"
#include "map.h"
#include "bind_map.h"

// --- C++ ---
CMap g_map(32, 32, 0);

void init_squirrel(HSQUIRRELVM v)
{
    Register_CMap(v); // your full binding

    // Expose shared map
    sq_pushroottable(v);
    sq_pushstring(v, _SC("getMap"), -1);
    sq_newclosure(v, [](HSQUIRRELVM v) -> SQInteger
                  {
        PushCMapInstance(v, &g_map);
        //CTreeRat::pushClassInstance(v, "CMap", &g_map);
        return 1; }, 0);
    sq_newslot(v, -3, SQFalse);
    sq_pop(v, 1);
}

void sq_test(CTreeRat &rat)
{
    const char *code = R"(
        print("Hello from script");
      //  throw "This is a test error";
    )";
    // rat.loadString(code);
    // rat.runScript("data/scripts/test.nut");

    rat.runScript("data/scripts/entity_test.nut");

    auto root = rat.root();
    auto configTable = rat.getOrCreateTable(root, "Config");
    rat.setValue(configTable, "isFullScreen", true);
    rat.setValue(configTable, "color", "red");
    auto result = rat.getValue<bool>(configTable, "isFullScreen");
    LOGI("result %d", result);

    Entity entity;
    entity.Move(10, 20);

    auto const1 = Sqrat::ConstTable(rat.vm())
                      .Const("Red", 1)
                      .Const("Green", 2)
                      .Const("Blue", 3);
    Sqrat::RootTable(rat.vm()).SetValue("Color", const1);

    Sqrat::Table configTable1(rat.getSlot(root, "Config"));
    rat.setValue(configTable1, "name", "John");
    rat.setValue(configTable1, "data", entity);
    rat.setValue(configTable1, "color", "red");
    rat.runScript("data/scripts/config.nut");
}

void map_test(CTreeRat &rat)
{
    g_map.set(5, 5, 100);
    init_squirrel(rat.vm());
    Register_CMap(rat.vm());
    PushCMapInstance(rat.vm(), &g_map);

    rat.runScript("data/scripts/map.nut");
}

int main()
{
    CTreeRat rat;
    registerGlobal(rat);
    // registerBinding(rat);

    map_test(rat);

    return 0;
}