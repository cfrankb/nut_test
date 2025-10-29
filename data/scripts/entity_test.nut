print("Script started");
local e = Entity();
e.Move(5, -3);
e.Damage(20);
print("Health is: " + e.GetHealth());
print("Name: " + e.name)