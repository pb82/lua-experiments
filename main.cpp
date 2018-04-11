#include <iostream>

#include "logger.h"
#include "nulldb.h"
#include "asyncqueue.h"
#include "pluginregistry.h"

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
    bool result = compileAction(logger, "function foo() a={}; while true do table.insert(a, 'test') end end\n function main() if pcall(foo) then print('success') else print('error') end end", &bytecode);
    if (result)
    {
        logger.info("Compilation successful. Bytecode size: %d", bytecode.size());
        persistence->addAction("hello", bytecode);
    }

    PluginRegistry registry(&logger);

    AsyncQueue::instance().setLogger(&logger);
    AsyncQueue::instance().setPersistence(persistence);

    // Stress testing
    for (int i = 0; i < 1; i++)
    {
        ActionBaton *act = new ActionBaton("hello");
        // Script needs to finish in 1 second or be killed
        act->timeout = 1000;

        // Script cannot use more than 100 kilobytes
        act->maxmem = 1;

        AsyncQueue::instance().submit(act);
    }

    AsyncQueue::instance().run();    
    delete persistence;
    return 0;
}
