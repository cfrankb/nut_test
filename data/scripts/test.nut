print("Running Squirrel script...\n");
printHello(); // Calls the native C++ function

local result = addNumbers(5, 7);
print("Sum is: " + result); // prints: Sum is: 12

greet("Frank", 3);