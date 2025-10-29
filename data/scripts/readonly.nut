local backing = {
    Red = "#ff0000"
};

local proxy = {};
proxy.__get = function(key) {
    print("GET: " + key);
    return backing[key];
}
proxy.__set = function(key, val) {
    throw "Read-only!";
}

::Const <- proxy;

print(Const.Red);      // Should print "GET: Red" then "#ff0000"
Const.Red = "#00ff00"; // Should throw "Read-only!"
