#include "pluginregistry.h"


PluginRegistry::PluginRegistry(Logger *logger) : logger(logger)
{
    logger->info("Loading plugins");
    loadPlugins();
}

void PluginRegistry::loadPlugins()
{
    uv_lib_t *lib = (uv_lib_t *) malloc(sizeof(uv_lib_t));
    uv_dlopen(SKELETON_PLUGIN_PATH, lib);

    int status;
    const char* pluginName;
    status = uv_dlsym(lib, "pluginName", (void **) &pluginName);
    logger->info("Loaded plugin: %s (%d)", pluginName, status);

    PluginSetup setup;
    status = uv_dlsym(lib, "setup", (void**) &setup);
    void *handle;
    setup(&handle);

    logger->info("Plugin value: %d (%d)", *((int *) handle), status);

    PluginDestroy destroy;
    status = uv_dlsym(lib, "destroy", (void**) &destroy);
    destroy(&handle);

    logger->info("Plugin destroyed: %d", status);
}
