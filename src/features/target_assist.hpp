#pragma once

#include <vector>
#include <windows.h>

#include "core/cheat_state.hpp"
#include "game/player_screen_position.hpp"

namespace features
{
    class TargetAssist
    {
    public:
        explicit TargetAssist(core::CheatState& state) : state_(state) {}

        [[nodiscard]] bool is_enabled() const { return state_.target_assist_enabled.load(); }
        void update(const std::vector<game::PlayerScreenPosition>& players);

    private:
        [[nodiscard]] HWND resolve_game_window();

        core::CheatState& state_;
        HWND game_window_{};
    };
}
