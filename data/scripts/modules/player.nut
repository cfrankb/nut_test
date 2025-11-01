// modules/player.nut
//local exports = {};
local exports = {};  // Fresh table created
//local name = "PlayerModule";

exports.name <- "PlayerModule";
exports.health <- 100;

exports.heal <- function(amount) {
    exports.health += amount;
    ::print("Healed to " + exports.health);
}

local map = ::g_map;
::print(map.title())

::print("exports table: " + exports);

return exports;