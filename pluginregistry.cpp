#include "pluginregistry.h"


PluginRegistry::PluginRegistry(Logger *logger, Config *config)
  : logger(logger)
  , config(config)
{
    logger->info("Loading plugins");
    loadPlugins();
}

plugin_ptr PluginRegistry::getPlugin(const char *name)
{
    return plugins[name];
}

void PluginRegistry::loadPlugins()
{
    for (std::string const& plugin : config->getConfiguredPlugins())
    {
        JSON::Value pluginConfig = config->getPluginConfig(plugin);
        if (pluginConfig.is(JSON::JSON_OBJECT))
        {
            loadPlugin(pluginConfig["file"].as<std::string>());
        } else
        {
            logger->error("Plugin config not an object (%s)", plugin.c_str());
        }
    }
}

void PluginRegistry::loadPlugin(std::string file)
{
    logger->info("Loading plugin from file: %s", file.c_str());

    int status = 0;
    uv_lib_t *lib = (uv_lib_t *) malloc(sizeof(uv_lib_t));
    status = uv_dlopen(file.c_str(), lib);

    if (status < 0)
    {
        std::cout << uv_dlerror(lib) << std::endl;
        exit(1);
    }

    PluginCreate create;
    status = uv_dlsym(lib, "create", (void **) &create);

    Plugin *plugin = create();
    plugin->setup(config->getPluginConfig(plugin->name()));
    plugins[plugin->name()] = std::shared_ptr<Plugin>(plugin);

    logger->info("Plugin loaded successfully");
}
