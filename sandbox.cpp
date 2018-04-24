#include "sandbox.h"

Sandbox::Sandbox()
{
    L = luaL_newstate();
    uv_rwlock_init(&lock);
    loadLibraries();

    // Set up the pointers
    SET_THIS;
    SET_ENV;
}

Sandbox::~Sandbox()
{
    lua_close(L);
}

void Sandbox::hook(lua_State *L, lua_Debug *)
{
    GET_ENV(env);
    GET_THIS(Sandbox, This);        
    UPDATE_MS(This, running);

    if (This->mslimit > 0 && This->running >= This->mslimit)
    {
        std::longjmp(*env, ErrTimeout);
    }

    if (This->kblimit > 0 && lua_gc(L, LUA_GCCOUNT, 0) >= This->kblimit)
    {
        std::longjmp(*env, ErrMemory);
    }        
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

    // Export the plugin call function
    lua_register(L, "call", Sandbox::callPlugin);
}

int Sandbox::callPlugin(lua_State *L)
{
    GET_THIS(Sandbox, This);
    uv_rwlock_wrlock(&This->lock);

    lua_settop(L, 3);
    luaL_checktype(L, 1, LUA_TSTRING);

    std::string plname = lua_tostring(L, 1);
    std::string action = lua_tostring(L, 2);

    JSON::Value payload;
    LuaTools::readObject(L, payload, 3);

    if(This->registry)
    {
        plugin_ptr plugin = This->registry->getPlugin(plname.c_str());
        JSON::Value result = plugin->call(action, payload);
        LuaTools::writeValue(L, result);
    }

    uv_rwlock_wrunlock(&This->lock);
    return 1;
}

RunCode Sandbox::runAction(std::string name, std::string &bytecode, std::string *msg, JSON::Value &result, JSON::Value &argument)
{
    int status, type;

    // Using hooks will have a performance impact. Only use them
    // if a constraint (time or memory) is actually set.
    if (mslimit > 0 || kblimit > 0)
    {
        // Init time tracking
        UPDATE_MS(this, started);
        lua_sethook(L, hook, LUA_MASKCOUNT, GRANULARITY);
    }

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

    int error = setjmp(env);
    switch(error)
    {
    case 0:
        break;
    case ErrTimeout:
        *msg = "Action `";
        msg->append(name);
        msg->append("` aborted after timeout");
        return ErrTimeout;
    case ErrMemory:
        *msg = "Action `";
        msg->append(name);
        msg->append("` aborted after violating memory constraints");
        return ErrMemory;
    default:
        *msg = "Unknown error";
        return ErrUnknown;
    }

    // Inject the artument
    LuaTools::writeValue(L, argument);

    // Second call to actually run the main function
    if ((status = lua_pcall(L, 1, 1, 0)) != LUA_OK)
    {
        *msg = lua_tostring(L, -1);
        return ErrMain;
    }

    LuaTools::readValue(L, result);
    return Success;
}
