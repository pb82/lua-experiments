#ifndef SANDBOX_H
#define SANDBOX_H

#include "lobj.h"
#include "logger.h"

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

    /**
     * @brief getInstructions
     * Return the value of the instruction counter
     * @return Number of instructions executed
     */
    int getInstructions() const;

private:
    int instructionCounter = 0;
};

#endif // SANDBOX_H
