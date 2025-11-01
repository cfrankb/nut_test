
let compiledscript=compilestring("::println(\"ciao Toto\")")
//run the script
compiledscript()

// or providing compile-time bindings:
let api = {function foo() {println("foo() called")}}
let compiledscript1=compilestring("foo()", "bindings_test", api)
compiledscript1()
