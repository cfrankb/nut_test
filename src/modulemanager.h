#pragma once
#include <sqrat.h>
#include <map>
#include <string>
#include <fstream>
#include <iostream>
#include <squirrel.h>
#include <sqstdaux.h> // for sqstd_seterrorhandlers

#include "logger.h"
#include "treerat.h"

class ScriptModuleManager
{
public:
    struct Module
    {
        std::string name;
        Sqrat::Table exports;
    };

    explicit ScriptModuleManager(HSQUIRRELVM vm) : vm(vm)
    {
        const char *className = "ScriptModuleManager";
        Sqrat::Class<ScriptModuleManager> mgrClass(vm, className);
        mgrClass
            .Ctor() // optional, only needed if scripts will construct it
            .Func("LoadModule", &ScriptModuleManager::LoadModule)
            .Func("Require", &ScriptModuleManager::Require)
            .Func("Unload", &ScriptModuleManager::Unload)
            .Func("ListModules", &ScriptModuleManager::ListModules);
        Sqrat::RootTable(vm).Bind(className, mgrClass);
    }

    bool LoadModule(const std::string &name, const std::string &path)
    {
        if (modules.count(name))
        {
            LOGE("Module already loaded: %s", name.c_str());
            return false;
        }

        std::string code = ReadFile(path);
        if (code.empty())
        {
            LOGE("Failed to read: %s", path.c_str());
            return false;
        }

        LOGI("code: %s", code.c_str());

        // Compile the script
        Sqrat::string errMsg;
        Sqrat::Script script(vm);

        bool result = script.CompileString(code.c_str(), errMsg, name);
        if (!result)
        {
            LOGE("Compile error: %s", errMsg.c_str());
            return false;
        }

        // CRITICAL: We need to manually execute to capture return value
        // script.Run() might not leave the return value on the stack

        // Push the compiled closure
        sq_pushobject(vm, script.GetObject());
        sq_pushroottable(vm); // Push 'this' (root table)

        // Call the script with return value capture
        if (SQ_FAILED(sq_call(vm, 1, SQTrue, SQTrue))) // SQTrue = want return value
        {
            LOGE("Script execution failed");
            sq_pop(vm, 1); // Pop closure
            return false;
        }

        LOGI("script executed");

        // Now the return value is on top of stack
        SQObjectType type = sq_gettype(vm, -1);
        LOGI("Return value type: %d (table=%d)", type, OT_TABLE);

        if (type != OT_TABLE)
        {
            LOGE("Script did not return a table, got type: %d", type);
            sq_pop(vm, 2); // Pop return value and closure
            return false;
        }

        // Get the exports table from stack
        HSQOBJECT exportsObj;
        sq_resetobject(&exportsObj);
        sq_getstackobj(vm, -1, &exportsObj);
        sq_addref(vm, &exportsObj);

        LOGI("retrieved exports");

        sq_pop(vm, 2); // Pop return value and closure

        // Wrap in Sqrat::Table
        Sqrat::Table exportsTable(exportsObj, vm);

        LOGI("wrapped");

        if (exportsTable.IsNull())
        {
            LOGE("Module missing exports: %s", name.c_str());
            sq_release(vm, &exportsObj);
            return false;
        }

        modules[name] = {name, exportsTable};
        LOGI("Module loaded successfully: %s", name.c_str());
        return true;
    }

    ///////////////////////////////////////////////////

    Sqrat::Table Require(const std::string &name)
    {
        if (!modules.count(name))
        {
            throw std::runtime_error("Module not loaded: " + name);
        }
        return modules[name].exports;
    }

    void Unload(const std::string &name)
    {
        modules.erase(name);
    }

    void ListModules() const
    {
        for (const auto &[name, mod] : modules)
        {
            std::cout << "Loaded: " << name << std::endl;
        }
    }

private:
    HSQUIRRELVM vm;
    std::map<std::string, Module> modules;

    std::string ReadFile(const std::string &path)
    {
        std::ifstream file(path);
        if (!file)
            return "";
        return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    }
};
