#include <emscripten.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lua/lauxlib.h"
#include "lua/lua.h"
#include "lua/lualib.h"

// Basic types and decls.
typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef int int32_t;
typedef unsigned int uint32_t;
typedef long long int64_t;
typedef unsigned long long uint64_t;

typedef unsigned long size_t;
typedef unsigned char byte;
typedef unsigned int uint;

#define NULL ((void*)0)

size_t script_size;
char* script_buffer;

size_t output_size;
char* output_buffer;

// init() is called from JS to allocate space for the script.
char* EMSCRIPTEN_KEEPALIVE init(size_t _script_size) {
  script_buffer = malloc(_script_size);
  script_size = _script_size;
  return script_buffer;
}

size_t EMSCRIPTEN_KEEPALIVE execute() {
  lua_State* L = luaL_newstate();
  luaL_openlibs(L);
  int error = luaL_loadstring(L, "hello !!") || lua_pcall(L, 0, 0, 0);

  if (error) {
    const char* error_string = lua_tostring(L, -1);
    lua_pop(L, 1);

    output_size = strlen(error_string);
    output_buffer = malloc(output_size);
    memcpy(output_buffer, error_string, output_size);
  }
  lua_close(L);

  return output_size;
}

char* EMSCRIPTEN_KEEPALIVE retrieve() { return output_buffer; }
