#pragma once
#include <sqrat.h>
#include <map>
#include <string>
#include <fstream>
#include <iostream>
#include <squirrel.h>
#include <sqstdaux.h>
#include <functional>
#include <exception>

#include "logger.h"
#include "treerat.h"

using EnvInjector = std::function<void(HSQUIRRELVM vm, Sqrat::Table &env)>;

class ScriptModuleManager
{
public:
    struct Module
    {
        std::string name;
        std::string path;
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

    bool LoadNativeModule(const std::string &name, SQInteger (*entryPoint)(HSQUIRRELVM))
    {
        if (modules.count(name))
        {
            LOGE("Module already loaded: %s", name.c_str());
            return false;
        }

        SQInteger result = entryPoint(vm);
        if (result != 1)
        {
            LOGE("Native module %s failed to load", name.c_str());
            return false;
        }

        HSQOBJECT exportsObj;
        sq_getstackobj(vm, -1, &exportsObj);
        sq_addref(vm, &exportsObj);
        Sqrat::Table exports(exportsObj, vm);
        sq_pop(vm, 1);

        // Call onLoad if present
        Sqrat::Function onLoad(exports, "onLoad");
        if (!onLoad.IsNull())
        {
            if (!onLoad.Execute(Sqrat::Table(vm)))
            {
                LOGE("onLoad failed for native module: %s", name.c_str());
            }
        }

        modules[name] = Module{name, "", exports};
        return true;
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

        modules[name] = {name, path, exportsObj};
        LOGI("Module loaded successfully: %s", name.c_str());
        return true;
    }

    ///////////////////////////////////////////////////

    Sqrat::Table Require(const std::string &name)
    {
        if (!modules.count(name))
        {
            LOGE("Module not loaded: %s", name.c_str());
        }
        return modules[name].exports;
    }

    void Unload(const std::string &name)
    {
        auto it = modules.find(name);
        if (it != modules.end())
        {
            Sqrat::Function onUnload(it->second.exports, "onUnload");
            if (!onUnload.IsNull())
            {
                if (!onUnload.Execute())
                    LOGE("onUnload error: %s", name.c_str());
            }
            modules.erase(it);
        }
    }

    void ListModules() const
    {
        for (const auto &[name, mod] : modules)
        {
            std::cout << "Loaded: " << name << std::endl;
        }
    }

    bool ReloadModule(const std::string &name)
    {
        auto it = modules.find(name);
        if (it == modules.end())
        {
            LOGE("Module not loaded: %s", name.c_str());
            return false;
        }

        std::string path = it->second.path; // Store path in Module struct
        Unload(name);
        return LoadModule(name, path);
    }

    void AddInjector(EnvInjector fn)
    {
        injectors.push_back(std::move(fn));
    }

private:
    HSQUIRRELVM vm;
    std::vector<EnvInjector> injectors;
    std::map<std::string, Module> modules;

    void ApplyInjectors(Sqrat::Table &env)
    {
        for (auto &fn : injectors)
            fn(vm, env);
    }

    std::string ReadFile(const std::string &path)
    {
        std::ifstream file(path);
        if (!file)
            return "";
        return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    }
};
