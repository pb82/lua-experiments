#ifndef PLUGINREGISTRY_H
#define PLUGINREGISTRY_H

#include <uv.h>
#include <iostream>
#include <cstring>

#include "logger.h"
#include "plugin.h"

#define SKELETON_PLUGIN_PATH "./skeleton.so"

typedef Plugin *(*PluginCreate)();

class PluginRegistry
{
public:

    PluginRegistry(Logger *logger, Config *config);

private:
    void loadPlugins();
    Logger *logger;
    Config *config;
};

#endif // PLUGINREGISTRY_H
