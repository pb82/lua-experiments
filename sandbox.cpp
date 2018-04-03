#include "sandbox.h"

Sandbox::Sandbox()
{
    lua_sethook(L, hook, LUA_MASKCOUNT, 1);
}

void Sandbox::incInstructions()
{
    this->instructionCounter++;
}

int Sandbox::getInstructions() const
{
    return this->instructionCounter;
}

void Sandbox::hook(lua_State *L, lua_Debug *ar)
{
    THIS(Sandbox)->incInstructions();
}

bool Sandbox::runAction(Logger &logger, std::string name, std::string &bytecode)
{
    int status;
    luaopen_base(L);

    if ((status = luaL_loadbuffer(L, bytecode.c_str(), bytecode.size(), name.c_str())) != LUA_OK)
    {
        logger.error(lua_tostring(L, -1));
        return false;
    }

    logger.info("Action `%s` loaded successfully", name.c_str());

    // Priming: run the script to set up all global vars
    if ((status = lua_pcall(L, 0, 0, 0)) != LUA_OK)
    {
        logger.error(lua_tostring(L, -1));
        return false;
    }

    // Push the main function on the stack
    lua_getglobal(L, "main");

    // Second call to actually run the main function
    if ((status = lua_pcall(L, 0, 0, 0)) != LUA_OK)
    {
        logger.error(lua_tostring(L, -1));
        return false;
    }

    return true;
}
