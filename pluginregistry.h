#ifndef PLUGINREGISTRY_H
#define PLUGINREGISTRY_H

#include <uv.h>
#include <iostream>
#include <cstring>

#include "logger.h"
#define SKELETON_PLUGIN_PATH "./skeleton.so"

typedef void (*PluginSetup)(void**);
typedef void (*PluginDestroy)(void**);

static void registerPlugin(const char* name)
{

}

class PluginRegistry
{
public:

    PluginRegistry(Logger *logger);

private:
    void loadPlugins();
    Logger *logger;
};

#endif // PLUGINREGISTRY_H
