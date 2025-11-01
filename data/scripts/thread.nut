function coroutine_test(a, b) {
    println($"{a} {b}")
    local ret = suspend("suspend 1")
    println($"the coroutine says {ret}")
    ret = suspend("suspend 2")
    println($"the coroutine says {ret}")
    ret = suspend("suspend 3")
    println($"the coroutine says {ret}")
    return "I'm done"
}

local coro = newthread(coroutine_test)

local susparam = coro.call("test","coroutine") //starts the coroutine

local i = 1
do {
    println($"suspend passed {susparam}")
    susparam = coro.wakeup("ciao "+i)
    ++i
} while(coro.getstatus()=="suspended")

println($"return passed {susparam}")


////////////////////////////

/*

function state1() {
    suspend("state1")
    return state2() //tail call
}

function state2() {
    suspend("state2")
    return state3() //tail call
}

function state3() {
    suspend("state3")
    return state1() //tail call
}

local statethread = newthread(state1)

println(statethread.call())

for (local j = 0; j < 10000; j++)
    println(statethread.wakeup())

    */