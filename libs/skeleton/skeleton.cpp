#include <iostream>
#include <string>

#include "../../plugin.h"

class SkeletonPlugin : public Plugin
{
public:
    void setup(JSON::Value &config)
    {
    }

    JSON::Value call(const std::string& action, JSON::Value& payload)
    {
        int x = payload["x"].as<int>();
        int y = payload["y"].as<int>();
        return x + y;
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

