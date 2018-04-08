#ifndef LOBJ_H
#define LOBJ_H

#include <map>
#include <csetjmp>

#include <lua.hpp>

#define THIS(type) LObj::getThisPtr<type>(L)
#define JMPB(type) LObj::getJmpBuf(L)

/**
 * @brief The LObj class
 * Helper class for integrating lua into C++
 */
class LObj
{
public:
    LObj();
    virtual ~LObj();

    /**
     * @brief getThisPtr Get the `this` pointer associated with
     * the given lua state.
     * @param l Lua state pointer
     * @return Pointer to an object that is associated with the given
     * lua state
     */
    template<typename T>
    static T* getThisPtr(lua_State *l)
    {
        return static_cast<T*>(LObj::ptrs[l]);
    }

    /**
     * @brief getJmpBuf Get the jmp_buf associated with the given
     * Lua state. The jmp_buf is used to invoke longjmp to kill a
     * script that ran for too long
     * @param l Lua state pointer
     * @return Pointer to the jmp_buf associated with the given
     * Lua state
     */
    static std::jmp_buf* getJmpBuf(lua_State *l)
    {
        return envs[l];
    }

    lua_State *L;
    std::jmp_buf env;

private:
    static std::map<lua_State *, LObj *> ptrs;
    static std::map<lua_State *, std::jmp_buf *> envs;
};

#endif // LOBJ_H
