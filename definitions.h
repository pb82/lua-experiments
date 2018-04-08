#ifndef DEFINES_H
#define DEFINES_H

#define SET_THIS \
    lua_pushlightuserdata(L, this); \
    lua_setglobal(L, "__SELF")

#define GET_THIS(TYPE, NAME) \
    lua_getglobal(L, "__SELF"); \
    luaL_checktype(L, -1, LUA_TLIGHTUSERDATA); \
    TYPE *NAME = static_cast<TYPE *>(lua_touserdata(L, -1))

#define SET_ENV \
    lua_pushlightuserdata(L, &env); \
    lua_setglobal(L, "__ENV")

#define GET_ENV(NAME) \
    lua_getglobal(L, "__ENV"); \
    luaL_checktype(L, -1, LUA_TLIGHTUSERDATA); \
    jmp_buf *NAME = static_cast<jmp_buf *>(lua_touserdata(L, -1))

#endif // DEFINES_H
