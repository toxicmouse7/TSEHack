#include "game/lua_game_state_reader.hpp"

namespace game
{
    std::vector<PlayerScreenPosition> LuaGameStateReader::read_players(int* lua_state) const
    {
        if (api_.is_number(lua_state, -1))
        {
            return {};
        }

        std::vector<PlayerScreenPosition> players;
        api_.push_nil(lua_state);
        while (api_.next(lua_state, -2) != 0)
        {
            const auto read_field = [this, lua_state](const char* name)
            {
                api_.get_field(lua_state, -1, name);
                const auto value = api_.to_number(lua_state, -1);
                api_.set_top(lua_state, -2);
                return value;
            };

            players.push_back({
                read_field("x"), read_field("y"), read_field("left"), read_field("top"),
                read_field("right"), read_field("bottom")
            });
            api_.set_top(lua_state, -2); // Pop the value; retain the key for lua_next.
        }
        return players;
    }
}
