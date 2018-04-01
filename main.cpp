#include <iostream>
#include "logger.h"

#include "config.h"
#include "nulldb.h"

using namespace std;

int bytecodeWriter(lua_State *, const void *p, size_t sz, void *ud)
{
    std::string *buffer = (std::string *)ud;
    buffer->append((const char *)p, sz);
    return 0;
}

bool compileAction(Logger &logger, const char* action, std::string *buffer)
{
    int status;
    lua_State *L = luaL_newstate();

    // Check if the chunk was compiled without errors
    if ((status = luaL_loadstring(L, action)) != LUA_OK)
    {
        logger.error(lua_tostring(L, -1));
        return false;
    }

    lua_dump(L, bytecodeWriter, buffer, true);
    lua_close(L);
    return true;
}

bool runAction(Logger &logger, Persistence *persistence, std::string name)
{
    int status = 0;
    std::string bytecode = persistence->getAction(name);
    lua_State *L = luaL_newstate();
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

    lua_close(L);
    return true;
}

Persistence *getPersistenceLayer(Config &config, Logger &logger)
{
    switch(config.getPersistenceType())
    {
    case NULLDB:
        logger.warn("Using in memory storage (Nulldb)");
        return new Nulldb();
    default:
        throw PersistenceError("No persistence layer configured");
    }
}

int main()
{
    Config config;
    config.loadConfig();

    Logger logger(config);
    logger.info("Config loaded");

    Persistence *persistence = getPersistenceLayer(config, logger);

    std::string bytecode;
    bool result = compileAction(logger, "function main() print('Hello from lua') end", &bytecode);
    if (result)
    {
        logger.info("Compilation successful. Bytecode size: %d", bytecode.size());
        persistence->addAction("hello", bytecode);
        runAction(logger, persistence, "hello");
    }

    delete persistence;
    return 0;
}
