local ModuleManager = dofile("module_manager.nut");

local player = ModuleManager.load("player", "modules/player.nut");
print(player.name); // PlayerModule
player.heal(25);    // Healed to 125

ModuleManager.list(); // Loaded: player

ModuleManager.unload("player");
