#ifndef CONFIG_H
#define CONFIG_H

#include <iostream>
#include <stdexcept>
#include <lua.hpp>

#include "definitions.h"
#include "json/PrettyPrinter.hpp"
#include "luatools.h"

#define CONFIG_FILE "./config.lua"

/**
 * @brief The PersistenceType enum
 * Lists the supported persistence types
 */
enum PersistenceType
{
    UNCONFIGURED = 1,
    NULLDB,
    MONGODB
};

/**
 * @brief The ConfigError class
 * Thrown when the config file is not found or when an error
 * occurs in the configuration script
 */
class ConfigError : public std::runtime_error
{
public:
    ConfigError(std::string message)
        : std::runtime_error(message)
    {
    }
};

/**
 * @brief The Config class
 * Load the configuration script and holds all configuration values
 * for other modules to use
 */
class Config
{
public:
    Config();
    ~Config();
    /**
     * @brief loadConfig Loads and executes the configuration script
     */
    void loadConfig();

    /**
     * @brief logger Lua callback to configure the logger
     * @param L Lua state pointer
     * @return an integer (0) indicating the number of results
     */
    static int logger(lua_State *L);

    /**
     * @brief nulldb Lua callback to configure the nulldb persistence
     * layer. This is the default no-persistence option, cache only.
     * @param L Lua state pointer
     * @return an integer (0) indicating the number of results
     */
    static int nulldb(lua_State *L);
    static int mongodb(lua_State *L);

    static int plugin(lua_State *L);
    static int pcdata(lua_State *L);

    // Get/Set log level
    void setLogLevel(const char *level);
    std::string& getLogLevel();

    // Get/Set log fancyness
    void setLogFancy(bool fancy);
    bool getLogFancy() const;

    // Get/Set log timestamp
    void setLogTimestamp(bool ts);
    bool getLogTimestamp() const;

    // Get/Set persistence driver
    void setPersistenceType(PersistenceType type);
    PersistenceType getPersistenceType();
    std::string& getMongoUrl();
    void setMongoUrl(const char *url);
    int getMongoPort();
    void setMongoPort(int port);

    // Get config for a specific plugin
    JSON::Value &getPluginConfig(std::string plugin);
    std::vector<std::string> getConfiguredPlugins();

private:
    // Logger config
    std::string logLevel = "info";
    bool logTimestamp = false;
    bool logFancy = false;

    // Persistence config
    PersistenceType persistenceType = UNCONFIGURED;
    std::string mongoUrl;
    int mongoPort;

    // Plugin config
    std::string currentPlugin = "";
    std::map<std::string, JSON::Value> pluginConfig;

    lua_State *L;
};

#endif // CONFIG_H
