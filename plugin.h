#ifndef PLUGIN_H
#define PLUGIN_H

#include "json/Value.hpp"

class Plugin
{
public:
    virtual ~Plugin() { }
    virtual void setup(JSON::Object &config) = 0;
};

#endif // PLUGIN_H
