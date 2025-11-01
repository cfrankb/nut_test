// module_manager.nut

local ModuleManager = {
    _modules = {},

    load = function(name, path) {
        if (this._modules.rawin(name)) {
            throw "Module '" + name + "' is already loaded.";
        }

        local env = {
            name = name,
            exports = {},
            require = this.require.bindenv(this)
        };

        local chunk = compilestring(readfile(path), path);
        chunk.setenv(env);
        chunk(); // execute module

        this._modules[name] <- env.exports;
        return env.exports;
    },

    require = function(name) {
        if (!this._modules.rawin(name)) {
            throw "Module '" + name + "' not loaded.";
        }
        return this._modules[name];
    },

    unload = function(name) {
        if (this._modules.rawin(name)) {
            this._modules.rawdelete(name);
        }
    },

    list = function() {
        foreach (k, v in this._modules) {
            print("Loaded: " + k);
        }
    }
};

return ModuleManager;
