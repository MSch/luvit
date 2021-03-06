#include <stdlib.h>
#include <assert.h>

#include "luv_tcp.h"

// Temporary hack: libuv should provide uv_inet_pton and uv_inet_ntop.
// Clean this up in tcp_wrap.cc too.
#if defined(__MINGW32__) || defined(_MSC_VER)
#include <inet_net_pton.h>
#include <inet_ntop.h>
# define uv_inet_pton ares_inet_pton
# define uv_inet_ntop ares_inet_ntop
#else // __POSIX__
# include <arpa/inet.h>
# define uv_inet_pton inet_pton
# define uv_inet_ntop inet_ntop
#endif


int luv_new_tcp (lua_State* L) {
  int before = lua_gettop(L);
  luv_ref_t* ref;

  uv_tcp_t* handle = (uv_tcp_t*)lua_newuserdata(L, sizeof(uv_tcp_t));
  uv_tcp_init(uv_default_loop(), handle);

  // Set metatable for type
  luaL_getmetatable(L, "luv_tcp");
  lua_setmetatable(L, -2);

  // Create a local environment for storing stuff
  lua_newtable(L);
  lua_setfenv (L, -2);

  // Store a reference to the userdata in the handle
  ref = (luv_ref_t*)malloc(sizeof(luv_ref_t));
  ref->L = L;
  lua_pushvalue(L, -1); // duplicate so we can _ref it
  ref->r = luaL_ref(L, LUA_REGISTRYINDEX);
  handle->data = ref;

  assert(lua_gettop(L) == before + 1);
  // return the userdata
  return 1;
}

int luv_tcp_bind (lua_State* L) {
  int before = lua_gettop(L);
  uv_tcp_t* handle = (uv_tcp_t*)luv_checkudata(L, 1, "tcp");
  const char* host = luaL_checkstring(L, 2);
  int port = luaL_checkint(L, 3);

  struct sockaddr_in address = uv_ip4_addr(host, port);

  if (uv_tcp_bind(handle, address)) {
    uv_err_t err = uv_last_error(uv_default_loop());
    return luaL_error(L, "tcp_bind: %s", uv_strerror(err));
  }

  assert(lua_gettop(L) == before);
  return 0;
}

int luv_tcp_bind6(lua_State* L) {
  int before = lua_gettop(L);
  uv_tcp_t* handle = (uv_tcp_t*)luv_checkudata(L, 1, "tcp");
  const char* host = luaL_checkstring(L, 2);
  int port = luaL_checkint(L, 3);

  struct sockaddr_in6 address = uv_ip6_addr(host, port);

  if (uv_tcp_bind6(handle, address)) {
    uv_err_t err = uv_last_error(uv_default_loop());
    return luaL_error(L, "tcp_bind6: %s", uv_strerror(err));
  }

  assert(lua_gettop(L) == before);
  return 0;
}

int luv_tcp_nodelay(lua_State* L) {
  int before = lua_gettop(L);
  uv_tcp_t* handle = (uv_tcp_t*)luv_checkudata(L, 1, "tcp");
  int value = lua_toboolean(L, 2);

  int status = uv_tcp_nodelay(handle, value);

  if (status == -1) {
    uv_err_t err = uv_last_error(uv_default_loop());
    return luaL_error(L, "tcp_nodelay: %s", uv_strerror(err));
  }

  assert(lua_gettop(L) == before);
  return 0;
}

int luv_tcp_getsockname(lua_State* L) {
  int before = lua_gettop(L);
  uv_tcp_t* handle = (uv_tcp_t*)luv_checkudata(L, 1, "tcp");
  int port;
  char ip[INET6_ADDRSTRLEN];
  int family;

  struct sockaddr_storage address;
  int addrlen = sizeof(address);

  if (uv_tcp_getsockname(handle, (struct sockaddr*)(&address), &addrlen)) {
    uv_err_t err = uv_last_error(uv_default_loop());
    return luaL_error(L, "tcp_getsockname: %s", uv_strerror(err));
  }

  family = address.ss_family;
  if (family == AF_INET) {
    struct sockaddr_in* addrin = (struct sockaddr_in*)&address;
    uv_inet_ntop(AF_INET, &(addrin->sin_addr), ip, INET6_ADDRSTRLEN);
    port = ntohs(addrin->sin_port);
  } else if (family == AF_INET6) {
    struct sockaddr_in6* addrin6 = (struct sockaddr_in6*)&address;
    uv_inet_ntop(AF_INET6, &(addrin6->sin6_addr), ip, INET6_ADDRSTRLEN);
    port = ntohs(addrin6->sin6_port);
  }

  lua_newtable(L);
  lua_pushnumber(L, port);
  lua_setfield(L, -2, "port");
  lua_pushnumber(L, family);
  lua_setfield(L, -2, "family");
  lua_pushstring(L, ip);
  lua_setfield(L, -2, "address");

  assert(lua_gettop(L) == before + 1);
  return 1;
}

int luv_tcp_getpeername(lua_State* L) {
  int before = lua_gettop(L);
  uv_tcp_t* handle = (uv_tcp_t*)luv_checkudata(L, 1, "tcp");
  int port;
  char ip[INET6_ADDRSTRLEN];
  int family;

  struct sockaddr_storage address;
  int addrlen = sizeof(address);

  if (uv_tcp_getpeername(handle, (struct sockaddr*)(&address), &addrlen)) {
    uv_err_t err = uv_last_error(uv_default_loop());
    return luaL_error(L, "tcp_getpeername: %s", uv_strerror(err));
  }

  family = address.ss_family;
  if (family == AF_INET) {
    struct sockaddr_in* addrin = (struct sockaddr_in*)&address;
    uv_inet_ntop(AF_INET, &(addrin->sin_addr), ip, INET6_ADDRSTRLEN);
    port = ntohs(addrin->sin_port);
  } else if (family == AF_INET6) {
    struct sockaddr_in6* addrin6 = (struct sockaddr_in6*)&address;
    uv_inet_ntop(AF_INET6, &(addrin6->sin6_addr), ip, INET6_ADDRSTRLEN);
    port = ntohs(addrin6->sin6_port);
  }

  lua_newtable(L);
  lua_pushnumber(L, port);
  lua_setfield(L, -2, "port");
  lua_pushnumber(L, family);
  lua_setfield(L, -2, "family");
  lua_pushstring(L, ip);
  lua_setfield(L, -2, "address");

  assert(lua_gettop(L) == before + 1);
  return 1;
}

int luv_tcp_connect(lua_State* L) {
  int before = lua_gettop(L);
  uv_tcp_t* handle = (uv_tcp_t*)luv_checkudata(L, 1, "tcp");

  const char* ip_address = luaL_checkstring(L, 2);
  int port = luaL_checkint(L, 3);

  struct sockaddr_in address = uv_ip4_addr(ip_address, port);

  luv_connect_ref_t* ref = (luv_connect_ref_t*)malloc(sizeof(luv_connect_ref_t));

  // Store a reference to the userdata
  ref->L = L;
  lua_pushvalue(L, 1);
  ref->r = luaL_ref(L, LUA_REGISTRYINDEX);

  // Give the connect_req access to this
  ref->connect_req.data = ref;

  if (uv_tcp_connect(&ref->connect_req, handle, address, luv_after_connect)) {
    uv_err_t err = uv_last_error(uv_default_loop());
    return luaL_error(L, "tcp_connect: %s", uv_strerror(err));
  }

  assert(lua_gettop(L) == before);
  return 0;
}

int luv_tcp_connect6(lua_State* L) {
  int before = lua_gettop(L);
  uv_tcp_t* handle = (uv_tcp_t*)luv_checkudata(L, 1, "tcp");

  const char* ip_address = luaL_checkstring(L, 2);
  int port = luaL_checkint(L, 3);

  struct sockaddr_in6 address = uv_ip6_addr(ip_address, port);

  luv_connect_ref_t* ref = (luv_connect_ref_t*)malloc(sizeof(luv_connect_ref_t));

  // Store a reference to the userdata
  ref->L = L;
  lua_pushvalue(L, 1);
  ref->r = luaL_ref(L, LUA_REGISTRYINDEX);

  // Give the connect_req access to this
  ref->connect_req.data = ref;

  if (uv_tcp_connect6(&ref->connect_req, handle, address, luv_after_connect)) {
    uv_err_t err = uv_last_error(uv_default_loop());
    return luaL_error(L, "tcp_connect6: %s", uv_strerror(err));
  }

  assert(lua_gettop(L) == before);
  return 0;
}

