#pragma once

#include <mutex>
#include <optional>
#include <string_view>
#include <unordered_map>

#include "core/cheat_state.hpp"
#include "game/lua_game_state_reader.hpp"
#include "features/target_assist.hpp"
#include "hooks/lua_api.hpp"

namespace hooks
{
    class LuaHooks
    {
    public:
        explicit LuaHooks(core::CheatState& state) : state_(state) {}
        bool install(features::TargetAssist& target_assist);
        void uninstall();

    private:
        static int __cdecl hooked_pcall(int* lua_state, int nargs, int nresults, int msgh);

        int on_pcall(int* lua_state, int nargs, int nresults, int msgh);
        [[nodiscard]] bool install_unlock_if_ready(int* lua_state);
        [[nodiscard]] std::optional<int> script_reference(int* lua_state, std::unordered_map<int*, int>& references,
                                                           std::string_view source, const char* name);

        inline static LuaHooks* active_instance_ = nullptr;
        LuaApi api_{};
        game::LuaGameStateReader game_state_reader_{api_};
        LuaPCall pcall_trampoline_{};
        core::CheatState& state_;
        features::TargetAssist* target_assist_{};
        std::mutex script_references_mutex_;
        std::unordered_map<int*, int> script_references_;
        std::unordered_map<int*, int> unlock_script_references_;
        bool pcall_hooked_ = false;
    };
}
