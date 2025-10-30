# TreeRat

This project is about creating a binding contract that can be used to automate generating C++ that actualize the runtime linkage.

# Auto Binding

Run the binder script to update `bind.cpp` and `bind.h`

```
$ python bin/binder.py
```

## Usage in C++ headers

| tag class   |                       |
| ----------- | --------------------- |
| @func       | member function       |
| @staticfunc | static function       |
| @prop       | property              |
| @var        | class variable        |
| @staticvar  | class static variable |
| @const      | class const variable  |

| definition tag |            |
| -------------- | ---------- |
| @struct        | structure  |
| @enum          | enumerator |

| global tag |                        |
| ---------- | ---------------------- |
| @export    | export global function |

```cpp
class CMap
{
public:
    CMap(uint16_t len = 0, uint16_t hei = 0, uint8_t t = 0);
    CMap(const CMap &map);
    ~CMap();
    uint8_t at(const int x, const int y) const; // @func
    void set(const int x, const int y, const uint8_t t); // @func
...

    inline const CStates &statesConst() const { return *m_states; }; // @func
    static uint16_t toKey(const uint8_t x, const uint8_t y);  // @staticfunc|uint16_t,uint8_t,uint8_t
    static uint16_t toKey(const Pos &pos); // @staticfunc:toKeyPos|uint16_t,const Pos&
    static Pos toPos(const uint16_t key);   // @staticfunc
    ...

```

## Parser syntax

Square brackets are used to denote optional parts of the tag annotation.

```
@tag[:alias][|parameters]
```

### @func @staticfunc

#### idiom

```cpp
int getSize(); // @func[:alias]
```

#### overload

```cpp
void resize(int, int); // @func|void,int,int
```

## @prop

Create properties. Declare the setter and getter functions to access them,

```cpp
int m_size; // @prop:attr,getter[,setter]
```

## @var @staticvar

variable exposed must have visibility in the binding context.

```cpp
int m_title; // @var[:alias]
```

## @struct

If optional parameters are declared, a constructor is addded.

```cpp
struct Pos // @struct[|int16_t,int16_t]
{
    int16_t x;
    int16_t y;
    ...
```

## @enum

```cpp
enum StateValue : uint16_t // @enum[:alias]
{
    TIMEOUT = 0x01,
    POS_ORIGIN = 0x02,
    POS_EXIT = 0x03,
    MAP_GOAL = 0x04,
    PAR_TIME = 0x05,
    ....
```

### @export

Export is used to expose C++ Quirrel functions to the scripting engine via TreeRat.

```cpp
    SQInteger greet(HSQUIRRELVM vm);   // @export[:alias]
```
