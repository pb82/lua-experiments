#ifndef SANDBOX_H
#define SANDBOX_H

#include <string>
#include <csetjmp>
#include <chrono>
#include <iostream>

#include <lua.hpp>

#include "definitions.h"

#define LUA_GLOBAL "_G"
#define GRANULARITY 100000

#define DISABLE_FN(TABLE, FN)   \
    lua_getglobal(L, TABLE);    \
    lua_pushstring(L, FN);      \
    lua_pushnil(L);             \
    lua_settable(L, -3);        \
    lua_pop(L, 1)

#define UPDATE_MS(Self,Target)                                              \
    Self->Target = std::chrono::duration_cast<std::chrono::milliseconds>(   \
        std::chrono::system_clock::now().time_since_epoch()                 \
    ).count() - Self->started

/**
 * @brief The RunCode enum
 * Information about the state of script execution
 */
enum RunCode
{    
    Success = 1,
    ErrTimeout,
    ErrBytecode,
    ErrPriming,
    ErrNoMain,
    ErrMain,
    ErrMemory,
    ErrUnknown
};

class Sandbox
{
public:
    Sandbox();

    /**
     * @brief runAction Run an action inside the sandbox
     * Run the given bytecode in a sandbox
     * @param name The name of the action for debug info
     * @param bytecode A string containing Lua bytecode
     * @return A code indicating if the action was run
     * successfully and if not, provides information about the
     * error
     */  
    RunCode runAction(std::string name, std::string &bytecode, std::string *msg);

    /**
     * @brief ms
     * Tracks the time a script is running in milliseconds. This
     * us updated after a certain number of instructions. Public
     * because this needs to be accesses in the static `hook`
     * handler.
     */
    long running = 0;
    long started = 0;

    /**
     * @brief limit Maximum script runtime
     * The maximum number of milliseconds the script is allowed to run.
     * If the script has not finished after this amount of time it will
     * be aborted. Zero means unlimited.
     */
    long mslimit = 0;
    int kblimit = 0;
private:
    /**
     * @brief loadLibraries Load liraries before running a script
     * Loads the Lua standard libraries but disables unsafe functions
     * for sandboxing concerns
     */
    void loadLibraries();

    /**
     * @brief hook Instruction counter
     * This hook is invoked when an instruction limit is set
     * @param L Lua state pointer
     * @param ar Lua debug info
     */
    static void hook(lua_State *L, lua_Debug *);

    lua_State *L;
    jmp_buf env;    
};

#endif // SANDBOX_H
