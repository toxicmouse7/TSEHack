#include "hooks/lua_api.hpp"

#include "platform/pe_exports.hpp"

namespace hooks
{
    LuaApi LuaApi::resolve_from_main_module()
    {
        return {
            platform::find_main_module_export<LuaLoadBuffer>("luaL_loadbuffer"),
            platform::find_main_module_export<LuaPCall>("lua_pcall"),
            platform::find_main_module_export<LuaLRef>("luaL_ref"),
            platform::find_main_module_export<LuaRawGetI>("lua_rawgeti"),
            platform::find_main_module_export<LuaSetTop>("lua_settop"),
            platform::find_main_module_export<LuaIsNumber>("lua_isnumber"),
            platform::find_main_module_export<LuaToNumber>("lua_tonumber"),
            platform::find_main_module_export<LuaToString>("lua_tolstring"),
            platform::find_main_module_export<LuaPushNil>("lua_pushnil"),
            platform::find_main_module_export<LuaNext>("lua_next"),
            platform::find_main_module_export<LuaGetField>("lua_getfield")
        };
    }

    bool LuaApi::is_complete() const
    {
        return load_buffer && pcall && ref && raw_get_i && set_top && is_number && to_number && to_string
            && push_nil && next && get_field;
    }
}
