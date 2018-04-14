#ifndef LUATOOLS_H
#define LUATOOLS_H

#include <lua.hpp>
#include "json/Value.hpp"

class LuaTools
{
public:
    static void readObject(lua_State *L, JSON::Object &obj, int tableindex);
};

#endif // LUATOOLS_H
