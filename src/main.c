#include <emscripten.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lua/lauxlib.h"
#include "lua/lua.h"
#include "lua/lualib.h"

#define OPEN_TAG "<?lua"
#define CLOSE_TAG "?>"

size_t input_buffer_size;
char* input_buffer;

size_t output_buffer_size;
char* output_buffer;

size_t echo_buffer_size;
char* echo_buffer;

int index_of(char* haystack, char* needle) {
  char* ptr = strstr(haystack, needle);
  if (ptr == NULL) {
    return -1;
  }
  return ptr - haystack;
}

char* EMSCRIPTEN_KEEPALIVE init(size_t script_size) {
  input_buffer_size = script_size;
  input_buffer = malloc(script_size);
  return input_buffer;
}

int l_echo(lua_State* L) {
  const char* message = luaL_checkstring(L, 1);
  int message_size = strlen(message);

  int new_size = echo_buffer_size + message_size;
  echo_buffer = realloc(echo_buffer, new_size * sizeof(char));
  memcpy(&echo_buffer[echo_buffer_size], message, message_size);
  echo_buffer_size = new_size;

  return 0;
}

void append_output(const char* data, int size) {
  int new_size = output_buffer_size + size;
  output_buffer = realloc(output_buffer, new_size * sizeof(char));
  memcpy(&output_buffer[output_buffer_size], data, size);
  output_buffer_size = new_size;
}

int execute_code_block(lua_State* L, const char* code, int size) {
  echo_buffer_size = 0;
  echo_buffer = NULL;

  int error = luaL_loadstring(L, code) || lua_pcall(L, 0, 0, 0);
  if (error) {
    return 1;
  }

  if (echo_buffer_size != 0) {
    append_output(echo_buffer, echo_buffer_size);
  }
  free(echo_buffer);

  return 0;
}

size_t EMSCRIPTEN_KEEPALIVE execute() {
  lua_State* L = luaL_newstate();
  luaL_openlibs(L);
  lua_pushcfunction(L, l_echo);
  lua_setglobal(L, "echo");

  output_buffer_size = 0;
  output_buffer = NULL;

  int curr = 0;
  int start_index;
#define remaining_input (&input_buffer[curr])
  while ((start_index = index_of(remaining_input, OPEN_TAG)) != -1) {
    append_output(remaining_input, start_index);
    curr += start_index + strlen(OPEN_TAG);  // advance past <?lua

    int end = index_of(remaining_input, CLOSE_TAG);
    if (end == -1) {
      return 0;
    }

    remaining_input[end] = '\0';  // create null-terminated string to execute
    int error = execute_code_block(L, remaining_input, end);
    if (error) {
      return 0;
    }

    curr += end + strlen(CLOSE_TAG);  // advance past ?>
  }
  append_output(remaining_input, input_buffer_size - curr);

  return output_buffer_size;
}

char* EMSCRIPTEN_KEEPALIVE retrieve() { return output_buffer; }
