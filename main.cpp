#include <iostream>
#include "logger.h"

#include "config.h"
#include "nulldb.h"
#include "sandbox.h"

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
    bool result = compileAction(logger, "function foo() for i=1,1000 do print('test') end end\n function main() if pcall(foo) then print('success') else print('error') end end", &bytecode);
    if (result)
    {
        logger.info("Compilation successful. Bytecode size: %d", bytecode.size());
        persistence->addAction("hello", bytecode);

        Sandbox s;
        s.runAction(logger, "hello", bytecode);
    }

    delete persistence;
    return 0;
}
