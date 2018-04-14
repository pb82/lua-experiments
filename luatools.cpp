#include "luatools.h"

void LuaTools::readObject(lua_State *L, JSON::Object &obj, int tableindex)
{
    lua_pushnil(L);
    while(lua_next(L, tableindex))
    {
        // The key must be a string
        // Convert to std::string directly because the
        // char pointer could be garbage collected

        std::string key;
        if (lua_isnumber(L, -2))
        {
            key = JSON::toString(lua_tonumber(L, -2));
        } else if (lua_isstring(L, -2))
        {
            key = lua_tostring(L, -2);
        } else
        {
            luaL_error(L, "Key must be a string or a number");
        }

        switch(lua_type(L, -1))
        {
        case LUA_TNUMBER:
            obj[key] = lua_tonumber(L, -1);
            break;
        case LUA_TSTRING:
            obj[key] = lua_tostring(L, -1);
            break;
        case LUA_TBOOLEAN:
            obj[key] = (bool) lua_toboolean(L, -1);
            break;
        case LUA_TTABLE:
            {
                JSON::Object nested;
                // The nested table will be at position -2 (from
                // the top) because of the nil that we push initially
                readObject(L, nested, -2);
                obj[key] = nested;
            }
            break;
        default:
            luaL_error(L, "Invalid config type");
        }
        lua_pop(L, 1);
    }
}
