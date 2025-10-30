print("hello map");

local map = g_map;
local tileID = map.at(5,5);
print("tileID: " + tileID)
print("title: " + map.title())
print("len: " + map.len())


local states = map.statesConst()
print("MSG0 - " + states.getS(StateValue.MSG0))
