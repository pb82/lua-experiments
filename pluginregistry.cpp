#include "pluginregistry.h"


PluginRegistry::PluginRegistry(Logger *logger, Config *config)
  : logger(logger)
  , config(config)
{
    logger->info("Loading plugins");
    loadPlugins();
}

void PluginRegistry::loadPlugins()
{
    int status = 0;
    uv_lib_t *lib = (uv_lib_t *) malloc(sizeof(uv_lib_t));
    status = uv_dlopen(SKELETON_PLUGIN_PATH, lib);

    PluginCreate create;
    status = uv_dlsym(lib, "create", (void **) &create);
    const char *err = uv_dlerror(lib);

    if (status < 0) {
        logger->error(err);
        exit(1);
    }

    Plugin *plugin = create();
    plugin->setup(config->getPluginConfig(plugin->name()));
    delete plugin;
}
