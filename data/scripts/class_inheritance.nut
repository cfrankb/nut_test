class BaseClass {
    constructor() {
        println("Base constructor")
    }
}

class ChildClass(BaseClass) {
    constructor() {
        base.constructor()
        println("Child constructor")
    }
}

let test = ChildClass()
