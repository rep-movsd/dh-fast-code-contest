/**
 * C wrapper around the lua implementation.
 *
 * feel free to move parts to and fro between the two.
 */
#include <string.h>
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
        fprintf(stderr, "error: can't init lua\n");
        exit(-1);
    }

    luaL_openlibs(L);

    if ( luaL_loadbuffer(L, lsrc, lsrc_len, "main.lua")  || lua_pcall(L, 0, LUA_MULTRET, 0) ) {
        fprintf(stderr, "error: can't load source\n");
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

    /**
     * the following block will convert the Rect * data into lua
     * tables to pass to the lua implementation.
     */
    for ( int i = 0; i < size; ++i ) {

        /**
         * we'll use this after we're done adding properties to the current
         * table -- (1)
         */
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

        // the stack already has the current index and table ( from (1) )
        lua_settable(L, -3);
    }

    lua_call(L, 1, 0);
}

void results(char *buffer)
{
    lua_getfield(L, 1, "results");
    if ( !lua_isfunction(L, -1) ) {
        fprintf(stderr, "error: results must be a function\n");
        exit(-1);
    }
    
    // call results()
    lua_call(L, 0, 1);

    // the function returned nothin
    if ( lua_isnil(L, -1) ) {
        lua_pop(L, 1);
        return;
    }

    // it returned something except a string
    if ( !lua_isstring(L, -1) ) {
        fprintf(stderr, "error: results must return a string or nil\n");
        exit(-1);
    }

    const char *val = lua_tostring(L, -1);
    
    // oh boy, no bounds checking. what could possibly go wrong?
    strcpy(buffer, val);

    // remove the returned value from stack
    lua_pop(L, 1);
}
