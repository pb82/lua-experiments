#include "config.h"

Config::Config()
{
    L = luaL_newstate();
    SET_THIS;       
}

Config::~Config()
{
    lua_close(L);
}

int Config::logger(lua_State *L)
{
    GET_THIS(Config, This);

    // Truncate to one argument and make sure its a table
    lua_settop(L, 1);
    luaL_checktype(L, 1, LUA_TTABLE);

    // Extract the fields from the table and push them on
    // the stack
    lua_getfield(L, 1, "level");
    lua_getfield(L, 1, "fancy");
    lua_getfield(L, 1, "timestamp");

    // Remove the table
    lua_remove(L, 1);    

    // Check the types of the items
    luaL_checktype(L, 1, LUA_TSTRING);
    luaL_checktype(L, 2, LUA_TBOOLEAN);
    luaL_checktype(L, 3, LUA_TBOOLEAN);

    bool ts = lua_toboolean(L, 3);
    bool fancy = lua_toboolean(L, 2);
    const char *level = lua_tostring(L, 1);

    This->setLogLevel(level);
    This->setLogFancy(fancy);
    This->setLogTimestamp(ts);

    // Clear stack
    lua_settop(L, 0);
    return 0;
}

int Config::nulldb(lua_State *L)
{
    GET_THIS(Config, This);

    // Truncate to one argument and make sure its a table
    lua_settop(L, 1);
    luaL_checktype(L, 1, LUA_TTABLE);

    // Remove the table
    lua_remove(L, 1);

    // Clear stack
    lua_settop(L, 0);

    // Throw if another persistency type is already configured
    if (This->getPersistenceType() != UNCONFIGURED)
    {
        luaL_error(L, "Only one persistence configuration allowed");
    }

    This->setPersistenceType(NULLDB);
    return 0;
}

int Config::plugin(lua_State *L)
{
    GET_THIS(Config, This);

    // Truncate to one argument and make sure its a table
    lua_settop(L, 1);
    luaL_checktype(L, 1, LUA_TSTRING);

    const char *plugin = lua_tostring(L, 1);
    This->currentPlugin = plugin;

    lua_remove(L, 1);
    lua_pushcfunction(L, Config::pcdata);

    return 1;
}

int Config::pcdata(lua_State *L)
{
    GET_THIS(Config, This);

    // Truncate to one argument and make sure its a table
    lua_settop(L, 1);
    luaL_checktype(L, 1, LUA_TTABLE);

    JSON::Value data;
    LuaTools::readValue(L, data);

    This->pluginConfig[This->currentPlugin] = data;
    return 0;
}

void Config::loadConfig()
{
    int status = 0;

    // Load the config script
    if ((status = luaL_loadfile(L, CONFIG_FILE)) != LUA_OK)
    {
        throw ConfigError("Config file not found");
    }

    // Register config callbacks
    lua_register(L, "Logger", &logger);
    lua_register(L, "Nulldb", &nulldb);
    lua_register(L, "Plugin", &plugin);

    // Run the config script
    if((status = lua_pcall(L, 0, 0, 0)) != LUA_OK)
    {
        throw ConfigError(lua_tostring(L, -1));
    }
}

void Config::setLogLevel(const char *level)
{
    logLevel = level;
}

std::string& Config::getLogLevel()
{
    return logLevel;
}

void Config::setLogFancy(bool fancy)
{
    logFancy = fancy;
}

bool Config::getLogFancy() const
{
    return logFancy;
}

void Config::setLogTimestamp(bool ts)
{
    logTimestamp = ts;
}

bool Config::getLogTimestamp() const
{
    return logTimestamp;
}

void Config::setPersistenceType(PersistenceType type)
{
    persistenceType = type;
}

PersistenceType Config::getPersistenceType()
{
    return persistenceType;
}

JSON::Value& Config::getPluginConfig(std::string plugin)
{    
    return pluginConfig[plugin];
}

std::vector<std::string> Config::getConfiguredPlugins()
{
    std::vector<std::string> result;
    for (auto const& pair : pluginConfig)
    {
        result.push_back(pair.first);
    }
    return result;
}
