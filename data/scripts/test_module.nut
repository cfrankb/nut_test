local player = g_man.Require("player")
print($"NUT Player Health: {player.health}")
player.heal(15)
print($"NUT Player Health: {player.health}")


local native = g_man.Require("native")
native.parse_args("1", "ABC", true)