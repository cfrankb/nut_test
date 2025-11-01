#pragma once
#include <sqrat.h>
#include <map>
#include <string>
#include <fstream>
#include <iostream>
#include <squirrel.h>
#include <sqstdaux.h> // for sqstd_seterrorhandlers
#include <functional>

#include "logger.h"
#include "treerat.h"

using EnvInjector = std::function<void(HSQUIRRELVM vm, Sqrat::Table &env)>;

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

        //  LOGI("code: %s", code.c_str());

        // Create environment
        Sqrat::Table env(vm);
        env.SetValue("name", name);
        env.SetValue("exports", Sqrat::Table(vm));

        // Inject globals
        ApplyInjectors(env);

        // Compile the script
        Sqrat::string errMsg;
        Sqrat::Script script(vm);
        bool result = script.CompileString(code.c_str(), errMsg, name, &env.GetObject());
        if (!result)
        {
            LOGE("Compile error: %s", errMsg.c_str());
            return false;
        }

        if (!script.Run(errMsg, &env))
        {
            LOGE("Runtime error: %s", errMsg.c_str());
            return false;
        }

        Sqrat::Object exportsObj = env["exports"];
        if (exportsObj.IsNull())
        {
            LOGE("Module missing exports: %s", name.c_str());
            return false;
        }

        modules[name] = {name, exportsObj};
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

    void AddInjector(EnvInjector fn)
    {
        injectors.push_back(std::move(fn));
    }

    void ApplyInjectors(Sqrat::Table &env)
    {
        for (auto &fn : injectors)
            fn(vm, env);
    }

private:
    HSQUIRRELVM vm;
    std::vector<EnvInjector> injectors;
    std::map<std::string, Module> modules;

    std::string ReadFile(const std::string &path)
    {
        std::ifstream file(path);
        if (!file)
            return "";
        return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    }
};
