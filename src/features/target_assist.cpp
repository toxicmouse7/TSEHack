#include "features/target_assist.hpp"

#include <limits>

namespace features
{
    HWND TargetAssist::resolve_game_window()
    {
        if (IsWindow(game_window_))
        {
            DWORD process_id{};
            GetWindowThreadProcessId(game_window_, &process_id);
            if (process_id == GetCurrentProcessId())
            {
                return game_window_;
            }
        }

        game_window_ = nullptr;
        struct WindowSearch
        {
            DWORD process_id;
            HWND window;
        } search = {.process_id = GetCurrentProcessId(), .window = nullptr};

        EnumWindows(
            [](const HWND window, const LPARAM parameter) -> BOOL
            {
                auto& search = *reinterpret_cast<WindowSearch*>(parameter);
                DWORD process_id{};
                GetWindowThreadProcessId(window, &process_id);

                if (process_id != search.process_id || !IsWindowVisible(window) || GetWindow(window, GW_OWNER) !=
                    nullptr)
                {
                    return TRUE;
                }

                search.window = window;
                return FALSE;
            },
            reinterpret_cast<LPARAM>(&search));

        game_window_ = search.window;
        return game_window_;
    }

    void TargetAssist::update(const std::vector<game::PlayerScreenPosition>& players)
    {
        if (!is_enabled()) return;
        if (state_.target_assist_require_activation_key.load() &&
            (GetAsyncKeyState(state_.target_assist_activation_key.load()) & 0x8000) == 0)
        {
            return;
        }

        const auto game_window = resolve_game_window();
        if (game_window == nullptr) return;
        if (GetForegroundWindow() != game_window) return;

        RECT rect{};
        GetClientRect(game_window, &rect);

        POINT cursor{};
        if (!GetCursorPos(&cursor) || !ScreenToClient(game_window, &cursor)) return;

        const game::PlayerScreenPosition* nearest_player = nullptr;
        auto nearest_distance_squared = (std::numeric_limits<double>::max)();
        for (const auto& player : players)
        {
            const auto x = player.x;
            const auto y = player.y;
            if (x < 0 || y < 0 || x > rect.right || y > rect.bottom)
            {
                continue;
            }

            const auto dx = x - cursor.x;
            const auto dy = y - cursor.y;
            const auto distance_squared = dx * dx + dy * dy;
            if (distance_squared < nearest_distance_squared)
            {
                nearest_distance_squared = distance_squared;
                nearest_player = &player;
            }
        }

        if (nearest_player)
        {
            POINT point{static_cast<int>(nearest_player->x), static_cast<int>(nearest_player->y)};
            ClientToScreen(game_window, &point);
            SetCursorPos(point.x, point.y);
        }
    }
}
