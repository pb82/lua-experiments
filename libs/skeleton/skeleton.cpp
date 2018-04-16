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
        return 0;
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

