#ifndef PLUGINREGISTRY_H
#define PLUGINREGISTRY_H

#include <uv.h>
#include <iostream>
#include <cstring>
#include <map>
#include <memory>

#include "logger.h"
#include "plugin.h"

#define SKELETON_PLUGIN_PATH "./skeleton.so"

typedef std::shared_ptr<Plugin> plugin_ptr;
typedef Plugin *(*PluginCreate)();

class PluginRegistry
{
public:

    PluginRegistry(Logger *logger, Config *config);
    plugin_ptr getPlugin(const char *name);

private:
    void loadPlugins();
    void loadPlugin(std::string file);

    Logger *logger;
    Config *config;

    std::map<std::string, plugin_ptr> plugins;
};

#endif // PLUGINREGISTRY_H
