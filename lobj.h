#ifndef LOBJ_H
#define LOBJ_H

#include <lua5.3/lua.hpp>
#include <map>

#define THIS(type) LObj::getThisPtr<type>(L)

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
    static T*const getThisPtr(lua_State *l)
    {
        return static_cast<T*>(LObj::ptrs[l]);
    }

    lua_State *L;

private:
    static std::map<lua_State *, LObj *> ptrs;
};

#endif // LOBJ_H
