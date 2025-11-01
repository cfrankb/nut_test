class Rect {
    constructor(w,h) {
        this.width = w
        this.height = h
    }
    x = 0
    y = 0
    width = null
    height = null
}

//Rect's constructor has 2 parameters so the class has to be 'called'
//with 2 parameters
let rc = Rect(100,100)

if (rc instanceof Rect) {
    println("It's a rect")
}
else {
    println("It isn't a rect")
}