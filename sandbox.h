#ifndef SANDBOX_H
#define SANDBOX_H

#include "lobj.h"
#include "logger.h"

#define LUA_GLOBAL "_G"
#define DISABLE_FN(TABLE, FN)   \
    lua_getglobal(L, TABLE);    \
    lua_pushstring(L, FN);      \
    lua_pushnil(L);             \
    lua_settable(L, -3);        \
    lua_pop(L, 1)


class Sandbox : public LObj
{
public:
    Sandbox();

    /**
     * @brief runAction Run an action inside the sandbox
     * Run the given bytecode in a sandbox
     * @param name The name of the action for debug info
     * @param bytecode A string containing Lua bytecode
     * @return A boolean indicating if the action was run
     * successfully
     */  
    bool runAction(Logger &logger, std::string name, std::string &bytecode);

    /**
     * @brief getInstructions
     * Return the value of the instruction counter
     * @return Number of instructions executed
     */
    int getInstructions() const;

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
    static void hook(lua_State *L, lua_Debug *ar);

    /**
     * @brief incInstructions
     * Increase the instruction counter by one
     */
    void incInstructions();

    int instructionCounter = 0;
};

#endif // SANDBOX_H
