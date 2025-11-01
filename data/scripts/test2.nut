print(::MyTable.greet("Anna"));

local a = 1
local b = a

local c = {
	function hello(name) {
		print("Hello "+ name)
	}
}
local d = c
local e = ::MyTable

d.hello("Mary")

print(e.greet("Salma"))

let as = 4
print("as=" + as)

let bs = @"This is a long text
that spans multiple lines.
"

print(bs)


let test = {
    a = 10
    b = function(x) { return this.a + x }
}

print("test.b(20)=" + test.b(20))


print("GkobalConst:"+GkobalConst)


//::Stuff <- Stuff;


local x = 123
local y = 567
print("x = {0}, y = {1}".subst(x, y))
print($"x = {x}, add = {1+2}")

local foo = 123
print($"\{ foo = {foo} \}") // will output: { foo = 123 }