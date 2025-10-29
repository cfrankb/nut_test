
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

int main()
{
    CTreeRat rat;
    registerGlobal(rat);

    const char *code = R"(
        print("Hello from script");
      //  throw "This is a test error";
    )";
    // rat.loadString(code);
    // rat.runScript("data/scripts/test.nut");

    registerBinding(rat);
    rat.runScript("data/scripts/entity_test.nut");
    // rat.reset();
    // rat.saveByteCode("data/scripts/test.nut", "out/hello.nutc");

    auto root = rat.root();
    auto configTable = rat.getOrCreateTable(root, "Config");
    rat.setValue(configTable, "isFullScreen", true);
    rat.setValue(configTable, "color", "red");
    auto result = rat.getValue<bool>(configTable, "isFullScreen");
    LOGI("result %d", result);

    Entity entity;
    entity.Move(10, 20);

    Sqrat::Table configTable1(rat.getSlot(root, "Config"));
    rat.setValue(configTable1, "name", "John");
    rat.setValue(configTable1, "data", entity);
    rat.setValue(configTable1, "color", "red");
    rat.runScript("data/scripts/config.nut");

    return 0;
}