#include "features/target_assist.hpp"

namespace features
{
    HWND TargetAssist::resolve_game_window()
    {
        if (game_window_ == nullptr || !IsWindow(game_window_))
        {
            game_window_ = FindWindow(nullptr, TEXT("The Showdown Effect"));
        }
        return game_window_;
    }

    void TargetAssist::update(const std::vector<game::PlayerScreenPosition>& players)
    {
        if (!is_enabled()) return;

        const auto game_window = resolve_game_window();
        if (game_window == nullptr) return;

        RECT rect{};
        GetClientRect(game_window, &rect);
        for (const auto& player : players)
        {
            const auto x = player.x;
            const auto y = player.y;
            if (x < 0 || y < 0 || x > rect.right || y > rect.bottom)
            {
                continue;
            }

            POINT point{static_cast<int>(x), static_cast<int>(y)};
            ClientToScreen(game_window, &point);
            SetCursorPos(point.x, point.y);
            break;
        }
    }
}
