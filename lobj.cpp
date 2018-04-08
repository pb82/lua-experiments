#include "lobj.h"

std::map<lua_State *, LObj *> LObj::ptrs = { };
std::map<lua_State *, std::jmp_buf *> LObj::envs = { };

LObj::LObj()
{    
    L = luaL_newstate();
    LObj::ptrs[L] = this;
    LObj::envs[L] = &env;
}

LObj::~LObj()
{    
    LObj::ptrs.erase(L);
    LObj::envs.erase(L);
    lua_close(L);
}
