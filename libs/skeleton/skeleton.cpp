#include <iostream>
#include <string>

#include "../../plugin.h"

class SkeletonPlugin : public Plugin
{
public:
    void setup(JSON::Value &config)
    {
        std::cout << "Config value: " << config["value"].as<int>() << std::endl;
    }


    JSON::Value call(const std::string& action, JSON::Value& payload)
    {
        if (action.compare("add") == 0) {
            double a = payload["a"].as<double>();
            double b = payload["a"].as<double>();
            return a + b;
        } else {
            return JSON::null;
        }
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

