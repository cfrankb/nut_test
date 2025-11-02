
#include "logger.h"

/*
# Debug build – show module name, no limit
-DSCRIPT_LOG_MODULE_NAME=1

# Release build – limit to 500 lines per VM lifetime
-DSCRIPT_LOG_RATE_LIMIT=500
*/

// Thread-safe, printf-style logging
static std::mutex log_mutex;

// -----------------------------------------------------------------
// Optional: rate-limit (0 = disabled)
// -----------------------------------------------------------------
#ifndef SCRIPT_LOG_RATE_LIMIT
#define SCRIPT_LOG_RATE_LIMIT 0
#endif

#if SCRIPT_LOG_RATE_LIMIT > 0
static std::atomic<int> log_counter{0};
#endif

// -----------------------------------------------------------------
// Optional: auto-prefix with the current module name
// -----------------------------------------------------------------
#ifndef SCRIPT_LOG_MODULE_NAME
#define SCRIPT_LOG_MODULE_NAME 0
#endif

#if SCRIPT_LOG_MODULE_NAME
static const char *get_current_module_name(HSQUIRRELVM vm)
{
    // Squirrel does not have a built-in "module name", but many engines
    // store it in the root table under a known key, e.g. "_MOD_NAME".
    sq_pushroottable(vm);
    sq_pushstring(vm, "_MOD_NAME", -1);
    if (SQ_SUCCEEDED(sq_get(vm, -2)))
    {
        const SQChar *name = nullptr;
        sq_getstring(vm, -1, &name);
        sq_pop(vm, 2);
        return name ? name : "?";
    }
    sq_pop(vm, 1);
    return "?";
}
#endif

// -----------------------------------------------------------------
// Public API – called from Squirrel
// -----------------------------------------------------------------
void script_log(const std::string &s)
{
#if SCRIPT_LOG_RATE_LIMIT > 0
    if (log_counter.fetch_add(1, std::memory_order_relaxed) >= SCRIPT_LOG_RATE_LIMIT)
        return; // silently drop
#endif

    std::lock_guard<std::mutex> lock(log_mutex);

#if SCRIPT_LOG_MODULE_NAME
    // You need a valid CTreeRat instance that owns the VM.
    // If you have a global or can pass it in, replace `nullptr` accordingly.
    const char *mod = get_current_module_name(/* rat.vm() */ nullptr);
    LOGI("[SCRIPT:%s] %s", mod, s.c_str());
#else
    LOGI("[SCRIPT] %s", s.c_str());
#endif
}

void script_error(const std::string &s)
{
    std::lock_guard<std::mutex> lock(log_mutex);

#if SCRIPT_LOG_MODULE_NAME
    const char *mod = get_current_module_name(/* rat.vm() */ nullptr);
    LOGE("[SCRIPT:%s] %s", mod, s.c_str());
#else
    LOGE("[SCRIPT] %s", s.c_str());
#endif
}

// Only in debug builds
#ifdef ENABLE_SCRIPT_DEBUG
void script_debug(const std::string &msg)
{
    LOGD("[DEBUG] %s", msg.c_str());
}
#else
void script_debug(const std::string &) {}
#endif
