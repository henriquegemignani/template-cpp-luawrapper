#include <cstdio>
#include "lua.hpp"
#include <tuple>


template <typename T>
struct ValueConverter;

template <>
struct ValueConverter<int>
{
    static void PushValue(lua_State* L, int v)
    {
        lua_pushinteger(L, v);
    }

    static int Unpack(lua_State* L, int idx)
    {
        return lua_tointeger(L, idx);
    }
};

template <typename T>
struct FunctionWrapper;

template <typename Ret, typename ...Args >
struct FunctionWrapper<Ret (*)(Args...)>
{
    template<std::size_t ...I>
    static int call_func(lua_State* L, std::index_sequence<I...>)
    {
        Ret(*func)(Args...) = static_cast<Ret(*)(Args...)>(lua_touserdata(L, lua_upvalueindex(1)));

        ValueConverter<Ret>::PushValue(L,
            func(ValueConverter<Args>::Unpack(L, I + 1)...));

        return 1;
    }

    static int lua_cfunc(lua_State* L)
    {
        return call_func(L, std::index_sequence_for<Args...>{});
    }

    static void PushFunction(lua_State* L, Ret(*f)(Args...))
    {
        lua_pushlightuserdata(L, f);
        lua_pushcclosure(L, lua_cfunc, 1);
    }
};

template <typename T>
void push_function_wrapper(lua_State* L, T func)
{
    FunctionWrapper<T>::PushFunction(L, func);
}

int double_arg(int x) {
    return x * 2;
}

int main()
{
    lua_State* L = lua_open();
    lua_gc(L, LUA_GCSTOP, 0);  /* stop collector during initialization */
    luaL_openlibs(L);  /* open libraries */
    lua_gc(L, LUA_GCRESTART, 0);

    push_function_wrapper(L, double_arg);
    lua_setglobal(L, "mah_function");

    luaL_loadfile(L, "main.lua");
    lua_call(L, 0, 0);

    lua_close(L);
    return 0;
}