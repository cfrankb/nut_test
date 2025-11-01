print("hello map");

local map = g_map;
local tileID = map.at(5,5);
print("tileID: " + tileID)
print("title: " + map.title())
print("len: " + map.len())

map.set(7,7,99);
print($"tileID at (7,7): {g_map.at(7,7)}")

local states = map.statesConst()
print("MSG0 - " + states.getS(StateValue.MSG0))

print("map type is: " + typeof(map))