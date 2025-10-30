#pragma once

class CTreeRat;

class Entity
{
public:
    Entity() : name("toto"), x(0), y(0), health(100), speed(4) {}

    void Move(int dx, int dy) // @func:Move
    {
        x += dx;
        y += dy;
    }

    void MoveDir(int aim) // @func:moveDir
    {
    }

    void Damage(int amount) // @func
    {
        health -= amount;
    }

    int GetHealth() const // @func
    {
        return health;
    }

    int GetSpeed() const
    {
        return speed;
    }

    void SetSpeed(int _speed)
    {
        speed = _speed;
    }

    static int getMaxHealth() // @staticfunc
    {
        return MAX_HEALTH;
    }

    int getX() { return x; }
    int getY() { return y; }

    enum Direction : int16_t
    {
        UP,
        DOWN,
        LEFT,
        RIGHT,
        MAX = RIGHT,
        NOT_FOUND = -1 // 0xffff
    };

private:
    const char *name;                    // @const
    int x;                               // @var:X @var:x
    int y;                               // @var
    int health;                          // @var
    int speed;                           // @prop:Speed,GetSpeed,SetSpeed
    inline static int MAX_HEALTH = 1024; // @staticvar

    friend void registerBinding(CTreeRat &rat);
};
