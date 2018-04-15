#ifndef PLUGIN_H
#define PLUGIN_H

#include "json/Value.hpp"

class Plugin
{
public:
    virtual ~Plugin() { }
    virtual std::string name() = 0;
    virtual void setup(JSON::Value &config) = 0;
    virtual JSON::Value call(const std::string& action, JSON::Value& payload) = 0;
};

#endif // PLUGIN_H
