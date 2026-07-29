#pragma once

#include <vector>

#include "game/player_screen_position.hpp"
#include "hooks/lua_api.hpp"

namespace game
{
    class LuaGameStateReader
    {
    public:
        explicit LuaGameStateReader(const hooks::LuaApi& api) : api_(api) {}

        [[nodiscard]] std::vector<PlayerScreenPosition> read_players(int* lua_state) const;

    private:
        const hooks::LuaApi& api_;
    };
}
