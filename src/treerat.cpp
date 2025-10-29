#include "treerat.h"
#include "logger.h"
#include <cstdarg>
#include <sstream>
#include <iostream>
#include <fstream>
#include <vector>

#include <sqstdmath.h>
#include <sqstdstring.h>
#include <sqstdsystem.h>
#include <sqstdio.h>
#include <sqstdaux.h>
#include <sqrat.h>

CTreeRat::CTreeRat()
{
    LOGI("Squirrel VM initialized");
    m_vm = sq_open(STACK_SIZE);
    sq_setprintfunc(m_vm, squirrel_print, squirrel_error); //  must be first
    sqstd_seterrorhandlers(m_vm);

    sq_pushroottable(m_vm);     //  push before registering libs
    sqstd_register_iolib(m_vm); //  defines print()
    sqstd_register_mathlib(m_vm);
    sqstd_register_stringlib(m_vm);
    sqstd_register_systemlib(m_vm);
    sq_pop(m_vm, 1);
}

CTreeRat::~CTreeRat()
{
    sq_close(m_vm);
}

void CTreeRat::squirrel_print(HSQUIRRELVM vm, const SQChar *s, ...)
{
    va_list args;
    va_start(args, s);
    printf("[SQUIRREL] ");
    vprintf(s, args);
    printf("\n");
    fflush(stdout); //  forces flush
    va_end(args);
}

void CTreeRat::squirrel_error(HSQUIRRELVM vm, const SQChar *s, ...)
{
    va_list args;
    va_start(args, s);
    fprintf(stderr, "[SQUIRREL ERROR]");
    vfprintf(stderr, s, args);
    fprintf(stderr, "\n");
    fflush(stderr); //  forces flush
    va_end(args);
}

SQInteger CTreeRat::MyFileWriter(SQUserPointer up, SQUserPointer data, SQInteger size)
{
    LOGI("received: %d bytes", size);
    if (size == 0)
        return 0;

    FILE *f = static_cast<FILE *>(up);
    size_t written = fwrite(data, size, 1, f);
    LOGI("written: %zu block of %d bytes", written, size);
    return (written == 1) ? size : SQ_ERROR;
}

bool CTreeRat::saveByteCode(const std::string &filepath, const std::string &outPath)
{
    // doesn't work
    std::ifstream in(filepath);
    if (!in)
    {
        LOGE("Script file not found: %s", filepath.c_str());
        return false;
    }
    std::string scriptText((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    LOGI("Script contents:\n%s", scriptText.c_str());

    FILE *out = fopen(outPath.c_str(), "wb");
    if (!out)
    {
        LOGE("Failed to open output file");
        return false;
    }

    HSQOBJECT env;
    sq_resetobject(&env);
    sq_pushroottable(m_vm);
    sq_getstackobj(m_vm, -1, &env);
    sq_pop(m_vm, 1);

    sq_pushroottable(m_vm);
    if (SQ_FAILED(sq_compile(m_vm, scriptText.c_str(), scriptText.length(), "inline", SQTrue, &env)))
    {
        sq_pop(m_vm, 1); // pop root table
        LOGE("Compile failed");
        return false;
    }

    LOGI("Stack after CompileString: %d", sq_gettop(m_vm));

    // Closure is now on top of the stack
    if (sq_gettop(m_vm) < 1)
    {
        LOGE("Stack is empty — no closure to inspect");
        return false;
    }

    if (sq_gettype(m_vm, -1) != OT_CLOSURE)
    {
        sq_pop(m_vm, 2); // pop closure + root
        LOGE("Top of stack is not a closure");
        return false;
    }

    HSQOBJECT obj;
    sq_getstackobj(m_vm, -1, &obj);
    LOGI("Closure type: %d", obj._type);
    LOGI("Stack after CompileString: %d", sq_gettop(m_vm));

    SQRESULT result = sq_writeclosure(m_vm, MyFileWriter, reinterpret_cast<SQUserPointer>(out));
    LOGI("sq_writeclosure returned: %d", result);
    if (SQ_FAILED(result))
    {
        LOGE("Writeclosure failed");
        sq_pop(m_vm, 2); // pop compiled closure
        fclose(out);
        return false;
    }
    fclose(out);

    sq_pop(m_vm, 2); // pop compiled closure
    return true;
}

bool CTreeRat::runScript(const std::string &filepath)
{
    // testPrint(vm);

    std::ifstream in(filepath);
    if (!in)
    {
        LOGE("Script file not found: %s", filepath.c_str());
        return false;
    }
    std::string contents((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    LOGI("Script contents:\n%s", contents.c_str());

    Sqrat::Script script(m_vm);
    Sqrat::string err;
    Sqrat::Table root = Sqrat::RootTable(m_vm);
    Sqrat::Object env(root); // wraps root safely

    if (!script.CompileString(contents.c_str(), err, filepath.c_str(), &env.GetObject()))
    {
        LOGE("Compile error:%s", err.c_str());
        return false;
    }
    else if (!script.Run(err, &env))
    {
        LOGE("Runtime error:%s", err.c_str());
        return false;
    }
    LOGI("Script finished");
    return true;
}

bool CTreeRat::loadString(const char *code)
{
    //////////////////////////////////////////////////
    Sqrat::Script script(m_vm);
    Sqrat::string err;

    //  Push root table and acquire a safe HSQOBJECT
    sq_pushroottable(m_vm); // stack: [root]
    HSQOBJECT envObj;
    sq_getstackobj(m_vm, -1, &envObj); // get root table object
    sq_pop(m_vm, 1);                   // clean up stack

    if (!script.CompileString(code, err, "inline_script", &envObj))
    {
        fprintf(stderr, "Compile error: %s\n", err.c_str());
        return false;
    }
    else if (!script.Run(err))
    {
        fprintf(stderr, "Runtime error: %s\n", err.c_str());
        return false;
    }
    return true;
}

void CTreeRat::registerFn(const char *name, SQFUNCTION fn)
{
    // Register native function
    sq_pushroottable(m_vm);
    sq_pushstring(m_vm, name, -1);
    sq_newclosure(m_vm, fn, 0);
    sq_newslot(m_vm, -3, SQFalse); // add to root table
    sq_pop(m_vm, 1);
}

void CTreeRat::reset()
{
    sq_close(m_vm);
    m_vm = sq_open(STACK_SIZE);
}

Sqrat::Table CTreeRat::root()
{
    return Sqrat::RootTable(m_vm);
}

bool CTreeRat::hasSlot(const Sqrat::Table &table, const std::string &key)
{
    sq_pushobject(m_vm, table.GetObject()); // push table
    sq_pushstring(m_vm, key.c_str(), -1);   // push key

    if (SQ_SUCCEEDED(sq_get(m_vm, -2)))
    {
        sq_pop(m_vm, 1); // pop value only
        sq_pop(m_vm, 1); // pop table
        return true;
    }

    sq_pop(m_vm, 1); // pop table (key already popped by sq_get)
    return false;
}

Sqrat::Table CTreeRat::getOrCreateTable(Sqrat::Table &parent, const std::string &key)
{
    if (!hasSlot(parent, key))
    {
        LOGI("new table: %s", key.c_str());
        Sqrat::Table subtable(m_vm);
        parent.Bind(key.c_str(), subtable); //  attach subtable to parent
        return subtable;
    }
    // Retrieve the table from parent[key]
    sq_pushobject(m_vm, parent.GetObject()); // push parent table
    sq_pushstring(m_vm, key.c_str(), -1);    // push key

    if (SQ_FAILED(sq_get(m_vm, -2)))
    {
        sq_pop(m_vm, 2); // clean up
        throw std::runtime_error("EnsureTable: failed to retrieve slot");
    }
    HSQOBJECT result;
    sq_getstackobj(m_vm, -1, &result);
    sq_pop(m_vm, 2); // pop value and parent
    return Sqrat::Table(result, m_vm);
}

Sqrat::Object CTreeRat::getSlot(const Sqrat::Table &table, const std::string &key)
{
    sq_pushobject(m_vm, table.GetObject()); // push the table
    sq_pushstring(m_vm, key.c_str(), -1);   // push the key
    if (SQ_SUCCEEDED(sq_get(m_vm, -2)))
    { // get the value
        HSQOBJECT obj;
        sq_getstackobj(m_vm, -1, &obj);  // extract HSQOBJECT
        sq_pop(m_vm, 2);                 // clean up
        return Sqrat::Object(obj, m_vm); // wrap it properly
    }
    sq_pop(m_vm, 2); // clean up
    throw std::runtime_error("Slot not found");
}

bool CTreeRat::pushClassInstance(HSQUIRRELVM vm, const std::string &className, void *ptr)
{
    // 1. Push the class (CMap) onto the stack
    sq_pushroottable(vm);
    sq_pushstring(vm, _SC(className.c_str()), -1);
    if (SQ_FAILED(sq_get(vm, -2)))
    {
        sq_pop(vm, 1);
        return false; // CMap not registered
    }

    // 2. Create a new instance (but don't construct!)
    sq_createinstance(vm, -1);
    sq_remove(vm, -2); // remove class
    sq_remove(vm, -2); // remove root

    // 3. Set the native pointer (no release hook!)
    sq_setinstanceup(vm, -1, ptr);

    // IMPORTANT: DO NOT set releasehook — C++ owns the object!
    // sq_setreleasehook(v, -1, ...);  // ← REMOVE THIS
    return true;
}