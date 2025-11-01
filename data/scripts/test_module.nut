local player = g_man.Require("player")
print($"NUT Player Health: {player.health}")
player.heal(15)
print($"NUT Player Health: {player.health}")
