#include "lobj.h"

std::map<lua_State *, LObj *> LObj::ptrs = { };

LObj::LObj()
{    
    L = luaL_newstate();
    LObj::ptrs[L] = this;
}

LObj::~LObj()
{
    LObj::ptrs.erase(L);
    lua_close(L);
}
