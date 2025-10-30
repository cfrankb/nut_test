
#include <squirrel.h>
#include <sqstdio.h>
#include <sqstdaux.h>
#include <sqrat.h>
#include <cstdio> // Required for printf
#include <cstdarg>
#include <sqstdmath.h>
#include <sqstdstring.h>
#include <sqstdsystem.h>
#include <sqrat/sqratMemberMethods.h>
#include "entity.h"
#include "logger.h"
#include "treerat.h"
#include "bind.h"
#include "methods.h"
#include "map.h"
#include "bind_map.h"
#include "rathelper.h"

CMap g_map(32, 32, 0);

void map_test(CTreeRat &rat)
{
    if (!g_map.read("data/maps/level01.dat"))
    {
        LOGE("cannot open map");
        return;
    }
    LOGI("title: %s", g_map.title());
    LOGI("g_map-->%p", &g_map);

    auto v = rat.vm();
    // Expose shared map
    Sqrat::RootTable(v).SetInstance("g_map", &g_map);

    rat.runScript("data/scripts/map.nut");
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

int main()
{
    CTreeRat rat;
    registerGlobal(rat);
    registerBinding(rat);

    map_test(rat);

    return 0;
}