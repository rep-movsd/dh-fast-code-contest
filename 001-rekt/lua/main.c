#include <stdio.h>
#include <stdlib.h>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "lsrc.h"

static lua_State *L = NULL;

#pragma pack(push)
#pragma pack(1)

struct Rect 
{
  float lx;
  float ly;
  float hx;
  float hy;
};

#pragma pack(pop)

void init(const char *filepath)
{
    L = lua_open();

    if ( L == NULL ) {
        fprintf(stderr, "error initializing lua\n");
        exit(-1);
    }

    luaL_openlibs(L);

    if ( luaL_loadbuffer(L, lsrc, lsrc_len, "main.lua")  || lua_pcall(L, 0, LUA_MULTRET, 0) ) {
        fprintf(stderr, "error loading source\n");
        exit(-1);
    }

    lua_getfield(L, -1, "init");

    if ( !lua_isfunction(L, -1) ) {
        fprintf(stderr, "error: init must be a function\n");
        exit(-1);
    }
    
    lua_pushstring(L, filepath);
    lua_call(L, 1, 0);
}

void run(struct Rect *rects, size_t size)
{
    lua_getfield(L, -1, "run");
    if ( !lua_isfunction(L, -1) ) {
        fprintf(stderr, "error: run must be a function\n");
        exit(-1);
    }

    lua_createtable(L, size, 0);

    for ( int i = 0; i < size; ++i ) {

        // we'll use this after we're done adding properties to the current table
        lua_pushnumber(L, i+1);
        lua_createtable(L, 0, 4);

        lua_pushstring(L, "lx");
        lua_pushnumber(L, rects[i].lx);
        lua_settable(L, -3);


        lua_pushstring(L, "ly");
        lua_pushnumber(L, rects[i].ly);
        lua_settable(L, -3);


        lua_pushstring(L, "hx");
        lua_pushnumber(L, rects[i].hx);
        lua_settable(L, -3);

        lua_pushstring(L, "hy");
        lua_pushnumber(L, rects[i].hy);
        lua_settable(L, -3);

        lua_settable(L, -3);
    }

    lua_call(L, 1, 0);
}

void results(char *buffer)
{
    ;
}
