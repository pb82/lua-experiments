#include "luatools.h"

void LuaTools::readValue(lua_State *L, JSON::Value &val)
{
    // Get the type of the item on top of the stack
    int type = lua_type(L, -1);
    switch(type)
    {
    case LUA_TTABLE:
        // -2 because readObject needs to push a nil value
        readObject(L, val, -2);
        break;
    case LUA_TBOOLEAN:
        val = (bool) lua_toboolean(L, -1);
        break;
    case LUA_TNUMBER:
        val = lua_tonumber(L, -1);
        break;
    case LUA_TSTRING:
        val = lua_tostring(L, -1);
        break;
    case LUA_TNIL:
        val = JSON::null;
        break;
    default:
        luaL_error(L, "readValue: unsupported type");
    }

    // Pop the value after reading
    lua_pop(L, 1);
}

void LuaTools::readObject(lua_State *L, JSON::Value &obj, int tableindex)
{
    lua_pushnil(L);
    while(lua_next(L, tableindex))
    {
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
                JSON::Value nested;
                // The nested table will be at position -2 (from
                // the top) because of the nil that we push initially
                readObject(L, nested, -2);
                obj[key] = nested;
            }
            break;
        default:
            luaL_error(L, "readObject: unsupported type");
        }
        lua_pop(L, 1);
    }
}

void LuaTools::writeObject(lua_State *L, JSON::Value &val)
{
    JSON::Object obj = val.as<JSON::Object>();

    lua_createtable(L, 0, obj.size());

    for (auto const& pair : obj)
    {
        std::string key = pair.first;
        JSON::Value value = pair.second;

        lua_pushstring(L, key.c_str());
        writeValue(L, value);
        lua_settable(L, -3);
    }
}

void LuaTools::writeArray(lua_State *L, JSON::Value &val)
{
    JSON::Array arr = val.as<JSON::Array>();

    lua_createtable(L, arr.size(), 0);

    for (unsigned int i = 1; i < arr.size() + 1; i++)
    {
        lua_pushnumber(L, i);
        writeValue(L, arr[i - 1]);
        lua_settable(L, -3);
    }
}

void LuaTools::writeValue(lua_State *L, JSON::Value &val)
{
    switch(val.getType())
    {
    case JSON::JSON_NULL:
        lua_pushnil(L);
        break;
    case JSON::JSON_BOOL:
        lua_pushboolean(L, (int) val.as<bool>());
        break;
    case JSON::JSON_NUMBER:
        lua_pushnumber(L, val.as<double>());
        break;
    case JSON::JSON_STRING:
        lua_pushstring(L, val.as<std::string>().c_str());
        break;
    case JSON::JSON_OBJECT:
        writeObject(L, val);
        break;
    case JSON::JSON_ARRAY:
        writeArray(L, val);
        break;
    default:
        luaL_error(L, "writeValue: unsupported type");
    }
}
