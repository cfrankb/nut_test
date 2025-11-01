
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
#include "modulemanager.h"

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
    rat.loadString(code);
    rat.runScript("data/scripts/test.nut");
    rat.runScript("data/scripts/test2.nut");
    rat.runScript("data/scripts/generator.nut");
    rat.runScript("data/scripts/class_rect.nut");
    rat.runScript("data/scripts/class_inheritance.nut");
    rat.runScript("data/scripts/thread.nut");
    rat.runScript("data/scripts/weakptr.nut");
    rat.runScript("data/scripts/entity_test.nut");
    rat.runScript("data/scripts/compiled.nut");
    rat.runScript("data/scripts/table.nut");

    auto root = rat.root();
    auto configTable = rat.getOrCreateTable(root, "Config");
    rat.setValue(configTable, "isFullScreen", true);
    rat.setValue(configTable, "color", "red");
    auto result = rat.getValue<bool>(configTable, "isFullScreen");
    LOGI("result %d", result);

    Entity entity;
    entity.Move(10, 20);

    auto const1 = Sqrat::ConstTable(rat.vm())
                      .Const("Red", "#ff0000")
                      .Const("Green", "#00ff00")
                      .Const("Blue", "#0000ff");
    Sqrat::RootTable(rat.vm()).Bind("Colors", const1);

    Sqrat::Table configTable1(rat.getSlot(root, "Config"));
    rat.setValue(configTable1, "name", "John");
    rat.setValue(configTable1, "data", entity);
    rat.setValue(configTable1, "color", "red");
    rat.runScript("data/scripts/config.nut");
}

void InjectGameGlobals(HSQUIRRELVM vm, Sqrat::Table &env)
{
    env.SetValue("g_map", g_map);
    env.SetValue("print", Sqrat::RootTable(vm)["print"]);
    // etc.
}

void module_test(CTreeRat &rat)
{
    ScriptModuleManager man(rat.vm());
    Sqrat::RootTable(rat.vm()).SetInstance("g_man", &man);
    man.AddInjector(InjectGameGlobals);
    man.LoadModule("player", "data/scripts/modules/player.nut");

    Sqrat::Table player = man.Require("player");
    Sqrat::Function heal(player, "heal");
    heal.Execute(25); // Healed to 125
    // int updatedHealth = player.GetSlot("health").Cast<int>(); // player.GetValue<int>("health");
    int updatedHealth = player["health"].Cast<int>();
    std::cout << "Updated health: " << updatedHealth << std::endl;

    heal.Execute(25); // Healed to 125
                      // int updatedHealth = player.GetSlot("health").Cast<int>(); // player.GetValue<int>("health");
    updatedHealth = player["health"].Cast<int>();
    std::cout << "Updated health: " << updatedHealth << std::endl;

    rat.runScript("data/scripts/test_module.nut");
}

int main()
{
    CTreeRat rat;
    registerGlobal(rat);
    registerBinding(rat);
    rat.setVerbose(false);

    // sq_test(rat);
    map_test(rat);
    module_test(rat);

    LOGI("vdfdfdf");

    return 0;
}