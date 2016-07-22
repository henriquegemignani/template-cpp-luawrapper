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
};

template <typename T>
struct FunctionWrapper;

template <typename Ret, typename ...Args >
struct FunctionWrapper<Ret (*)(Args...)>
{
    static std::tuple<Args...> get_arguments_from_lua(lua_State* L)
    {
        return std::tuple<Args...>();
    }

    template<std::size_t ...I>
    static Ret call_func(Ret(*func)(Args...), std::tuple<Args...>&& params, std::index_sequence<I...>)
    {
        return func(std::get<I>(params)...);
    }

    static int lua_cfunc(lua_State* L)
    {
        void* f_void = lua_touserdata(L, lua_upvalueindex(1));
        Ret(*f)(Args...) = static_cast<Ret(*)(Args...)>(f_void);

        ValueConverter<Ret>::PushValue(L,
            call_func(
                f,
                get_arguments_from_lua(L),
                std::index_sequence_for<Args...>{}));

        return 1;
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