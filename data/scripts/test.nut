print("Running Squirrel script...\n");
printHello(); // Calls the native C++ function

local result = addNumbers(5, 7);
print("Sum is: " + result); // prints: Sum is: 12

greet("Frank", 3);




// Define a table named 'MyTable'
local MyTable = {
    // Define a function 'greet' within MyTable
    function greet(name) {
        return "Hello, " + name + " from MyTable!";
    },

    // Define another function 'add' within MyTable
    function add(a, b) {
        return a + b;
    }
};


print(MyTable.greet("Sonya"));



for (local i =0; i < 10; ++i) {
	print("loop"+i);
}

::MyTable <- MyTable; 
::MyTable = MyTable;

class Box {
	constructor() {		
	}
    function whatIs() {
        println("This is a box")
    }	
}

let box = Box()
box.whatIs()

print(format("0x%.2x", 64))



global enum Stuff {
  first, //this will be 0
  second, //this will be 1
  third //this will be 2
}

global const GkobalConst = 4;
