#pragma once

class CTreeRat;

class Entity
{
public:
    Entity() : name("toto"), x(0), y(0), health(100), speed(4) {}

    // @func:Move
    void Move(int dx, int dy)
    {
        x += dx;
        y += dy;
    }

    // @func:moveDir
    void MoveDir(int aim)
    {
    }

    // @func
    void Damage(int amount)
    {
        health -= amount;
    }

    // @func
    int GetHealth() const
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

    // @staticfunc
    static int getMaxHealth()
    {
        return MAX_HEALTH;
    }

    int getX() { return x; }
    int getY() { return y; }

private:
    // @const
    const char *name;
    // @var:X
    int x;
    // @var
    int y;
    // @var
    int health;
    // @prop:Speed,GetSpeed,SetSpeed
    int speed;
    // @staticvar
    inline static int MAX_HEALTH = 1024;

    friend void registerBinding(CTreeRat &rat);
};
