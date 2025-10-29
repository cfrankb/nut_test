#pragma once

#include <sqrat.h>
#include <squirrel.h>
#include <string>

class CTreeRat
{
public:
    CTreeRat();
    ~CTreeRat();
    bool runScript(const std::string &filepath);
    bool saveByteCode(const std::string &filepath, const std::string &dest);
    bool loadString(const char *s);
    inline HSQUIRRELVM &vm() { return m_vm; }
    void registerFn(const char *name, SQFUNCTION fn);
    void reset();
    Sqrat::Table root();
    template <typename T>
    void setValue(Sqrat::Table &table, const SQChar *name, const T &val)
    {
        // auto &wm = m_vm;
        sq_pushobject(m_vm, table.GetObject());
        sq_pushstring(m_vm, name, -1);
        Sqrat::PushVar(m_vm, val);
        sq_newslot(m_vm, -3, false);
        sq_pop(m_vm, 1);
    }

    template <typename T>
    T getValue(Sqrat::Table &table, const SQChar *name)
    {
        sq_pushobject(m_vm, table.GetObject()); // push table
        sq_pushstring(m_vm, name, -1);          // push key

        if (SQ_FAILED(sq_get(m_vm, -2)))
        {
            sq_pop(m_vm, 1); // pop table
            throw std::runtime_error("getValue: key not found");
        }

        HSQOBJECT obj;
        sq_getstackobj(m_vm, -1, &obj); // extract value
        sq_pop(m_vm, 2);                // pop value and table

        Sqrat::Object wrapped(obj, m_vm);
        return wrapped.Cast<T>(); // cast to desired type
    }
    bool hasSlot(const Sqrat::Table &table, const std::string &key);
    Sqrat::Object getSlot(const Sqrat::Table &table, const std::string &key);
    Sqrat::Table getOrCreateTable(Sqrat::Table &parent, const std::string &key);
    static bool pushClassInstance(HSQUIRRELVM vm, const std::string &className, void *ptr);

private:
    enum
    {
        STACK_SIZE = 1024,
    };
    static void squirrel_print(HSQUIRRELVM vm, const SQChar *s, ...);
    static void squirrel_error(HSQUIRRELVM vm, const SQChar *s, ...);
    static SQInteger MyFileWriter(SQUserPointer up, SQUserPointer data, SQInteger size);
    HSQUIRRELVM m_vm;
};