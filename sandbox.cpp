#include "sandbox.h"

Sandbox::Sandbox()
{
    L = luaL_newstate();
    loadLibraries();

    // Set up the pointers
    SET_THIS;
    SET_ENV;

    lua_sethook(L, hook, LUA_MASKCOUNT, 100000);
}

void Sandbox::hook(lua_State *L, lua_Debug *)
{
    GET_ENV(env);
    std::longjmp(*env, ErrTimeout);
}

void Sandbox::loadLibraries()
{
    // Load safe libraries
    luaL_requiref(L, LUA_GLOBAL,        luaopen_base,       true);
    luaL_requiref(L, LUA_STRLIBNAME,    luaopen_string,     true);
    luaL_requiref(L, LUA_MATHLIBNAME,   luaopen_math,       true);
    luaL_requiref(L, LUA_UTF8LIBNAME,   luaopen_utf8,       true);
    luaL_requiref(L, LUA_COLIBNAME,     luaopen_coroutine,  true);
    luaL_requiref(L, LUA_TABLIBNAME,    luaopen_table,      true);

    // The OS lib has some useful functions for data and time we
    // want to retain these and only disable the dangerous ones
    luaL_requiref(L, LUA_OSLIBNAME,     luaopen_os,         true);

    // Disable unsafe functions
    DISABLE_FN(LUA_GLOBAL,              "dofile");
    DISABLE_FN(LUA_GLOBAL,              "collectgarbage");
    DISABLE_FN(LUA_GLOBAL,              "load");
    DISABLE_FN(LUA_GLOBAL,              "loadfile");
    DISABLE_FN(LUA_GLOBAL,              "require");
    DISABLE_FN(LUA_GLOBAL,              "module");
    DISABLE_FN(LUA_GLOBAL,              "module");
    DISABLE_FN(LUA_OSLIBNAME,           "execute");
    DISABLE_FN(LUA_OSLIBNAME,           "exit");
    DISABLE_FN(LUA_OSLIBNAME,           "getenv");
    DISABLE_FN(LUA_OSLIBNAME,           "remove");
    DISABLE_FN(LUA_OSLIBNAME,           "setlocale");
    DISABLE_FN(LUA_OSLIBNAME,           "tmpname");
}

RunCode Sandbox::runAction(std::string name, std::string &bytecode, std::string *msg)
{
    int status, type;

    if ((status = luaL_loadbuffer(L, bytecode.c_str(), bytecode.size(), name.c_str())) != LUA_OK)
    {
        *msg = lua_tostring(L, -1);
        return ErrBytecode;
    }

    // Priming: run the script to set up all global vars
    if ((status = lua_pcall(L, 0, 0, 0)) != LUA_OK)
    {
        *msg = lua_tostring(L, -1);
        return ErrPriming;
    }

    // Push the main function on the stack
    if((type = lua_getglobal(L, "main")) != LUA_TFUNCTION)
    {
        *msg = lua_tostring(L, -1);
        return ErrNoMain;
    }

    if (setjmp(env) > 0)
    {
        *msg = "Action `";
        msg->append(name);
        msg->append("` aborted after timeout");
        return ErrTimeout;
    }

    // Second call to actually run the main function
    if ((status = lua_pcall(L, 0, 0, 0)) != LUA_OK)
    {
        *msg = lua_tostring(L, -1);
        return ErrMain;
    }

    return Success;
}
