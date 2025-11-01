local t = {}
local a = ["first","second","third"]
//creates a weakref to the array and assigns it to a table slot
t.thearray <- a.weakref()

print(t.thearray[0])

a = 123

print(typeof(t.thearray))


let tt = {}
let weakobj = tt.weakref()

print(typeof(weakobj))

print(typeof(weakobj.ref()))

