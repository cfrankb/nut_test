# TreeRat

This project is about creating a binding contract that can be used to automate generating C++ that actualize the runtime linkage.

# Auto Binding

Run the binder script to update `bind.cpp` and `bind.h`

```
$ python bin/binder.py
```

Usage in C++ headers

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


struct Pos // @struct|int16_t,int16_t
{
    int16_t x;
    int16_t y;
....

```
