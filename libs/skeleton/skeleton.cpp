#include <iostream>
#include <string>

#include "../../plugin.h"

class SkeletonPlugin : public Plugin
{
public:
    void setup(JSON::Object &config)
    {
        std::cout << "Config value: " << config["value"].as<int>() << std::endl;
    }

    std::string name()
    {
        return "skeleton";
    }
};

extern "C" Plugin *create()
{
    return new SkeletonPlugin;
}

