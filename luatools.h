#ifndef LUATOOLS_H
#define LUATOOLS_H

#include <lua.hpp>
#include "json/Value.hpp"

class LuaTools
{
public:
    static void readValue(lua_State *L, JSON::Value &val);
    static void readObject(lua_State *L, JSON::Value &obj, int tableindex);

    static void writeValue(lua_State *L, JSON::Value &val);
    static void writeObject(lua_State *L, JSON::Value &val);
};

#endif // LUATOOLS_H
