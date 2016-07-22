#include <cstdio>
#include "lua.hpp"

int mah_function(lua_State* L)
{
    lua_pushinteger(L, 5);
    return 1;
}

int main()
{
    lua_State* L = lua_open();
    lua_gc(L, LUA_GCSTOP, 0);  /* stop collector during initialization */
    luaL_openlibs(L);  /* open libraries */
    lua_gc(L, LUA_GCRESTART, 0);

    lua_pushcfunction(L, mah_function);
    lua_setglobal(L, "mah_function");

    luaL_loadfile(L, "main.lua");
    lua_call(L, 0, 0);

    lua_close(L);
    return 0;
}