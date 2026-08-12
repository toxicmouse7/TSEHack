#include "hooks/lua_hooks.hpp"

#include <utility>
#include <vector>

#include <MinHook.h>

#include "features/target_script.hpp"
#include "features/unlock_script.hpp"

namespace hooks
{
    namespace
    {
        // The game exposes the Lua 5.1-style lua_pcall API, whose registry has this index.
        constexpr int lua_registry_index = -10000;
    }

    bool LuaHooks::install(features::TargetAssist& target_assist)
    {
        api_ = LuaApi::resolve_from_main_module();
        if (!api_.is_complete() || active_instance_ != nullptr)
        {
            return false;
        }

        target_assist_ = &target_assist;
        active_instance_ = this;
        if (MH_CreateHook(reinterpret_cast<LPVOID>(api_.pcall), reinterpret_cast<LPVOID>(hooked_pcall),
                          reinterpret_cast<LPVOID*>(&pcall_trampoline_)) != MH_OK)
        {
            uninstall();
            return false;
        }
        pcall_hooked_ = true;
        if (MH_EnableHook(reinterpret_cast<LPVOID>(api_.pcall)) != MH_OK)
        {
            uninstall();
            return false;
        }
        return true;
    }

    void LuaHooks::uninstall()
    {
        if (pcall_hooked_)
        {
            MH_DisableHook(reinterpret_cast<LPVOID>(api_.pcall));
            MH_RemoveHook(reinterpret_cast<LPVOID>(api_.pcall));
            pcall_hooked_ = false;
        }
        pcall_trampoline_ = nullptr;
        target_assist_ = nullptr;
        std::lock_guard lock(script_references_mutex_);
        script_references_.clear();
        unlock_script_references_.clear();
        if (active_instance_ == this)
        {
            active_instance_ = nullptr;
        }
    }

    int __cdecl LuaHooks::hooked_pcall(int* lua_state, const int nargs, const int nresults, const int msgh)
    {
        return active_instance_->on_pcall(lua_state, nargs, nresults, msgh);
    }

    int LuaHooks::on_pcall(int* lua_state, const int nargs, const int nresults, const int msgh)
    {
        const auto unlock_pending = state_.unlock_requested.load() && !state_.unlock_installed.load();
        if (!target_assist_ || (!target_assist_->is_enabled() && !state_.esp_enabled.load() && !unlock_pending))
        {
            return pcall_trampoline_(lua_state, nargs, nresults, msgh);
        }

        if (unlock_pending && install_unlock_if_ready(lua_state))
        {
            state_.unlock_installed.store(true);
        }

        if (!target_assist_->is_enabled() && !state_.esp_enabled.load())
        {
            return pcall_trampoline_(lua_state, nargs, nresults, msgh);
        }

        const auto reference = script_reference(lua_state, script_references_, features::target_script,
                                                "@showdown_target_assist");
        if (!reference)
        {
            return pcall_trampoline_(lua_state, nargs, nresults, msgh);
        }

        api_.raw_get_i(lua_state, lua_registry_index, *reference);
        if (pcall_trampoline_(lua_state, 0, 1, 0) != 0)
        {
            api_.set_top(lua_state, -2);
            return pcall_trampoline_(lua_state, nargs, nresults, msgh);
        }

        const auto players = game_state_reader_.read_players(lua_state);
        if (state_.esp_enabled.load())
        {
            std::vector<core::EspBox> boxes;
            boxes.reserve(players.size());
            for (const auto& player : players)
            {
                if (player.left < player.right && player.top < player.bottom)
                {
                    boxes.push_back({static_cast<float>(player.left), static_cast<float>(player.top),
                                     static_cast<float>(player.right), static_cast<float>(player.bottom)});
                }
            }
            state_.set_esp_boxes(std::move(boxes));
        }
        target_assist_->update(players);
        api_.set_top(lua_state, -2);
        return pcall_trampoline_(lua_state, nargs, nresults, msgh);
    }

    bool LuaHooks::install_unlock_if_ready(int* lua_state)
    {
        const auto reference = script_reference(lua_state, unlock_script_references_, features::unlock_script,
                                                "@showdown_unlock_all");
        if (!reference)
        {
            return false;
        }

        api_.raw_get_i(lua_state, lua_registry_index, *reference);
        if (pcall_trampoline_(lua_state, 0, 1, 0) != 0)
        {
            api_.set_top(lua_state, -2);
            return false;
        }

        const auto installed = api_.is_number(lua_state, -1) && api_.to_number(lua_state, -1) == 1.0;
        api_.set_top(lua_state, -2);
        return installed;
    }

    std::optional<int> LuaHooks::script_reference(int* lua_state, std::unordered_map<int*, int>& references,
                                                   const std::string_view source, const char* name)
    {
        std::lock_guard lock(script_references_mutex_);
        if (const auto it = references.find(lua_state); it != references.end())
        {
            return it->second;
        }

        if (api_.load_buffer(lua_state, source.data(), source.size(), name) != 0)
        {
            api_.set_top(lua_state, -2); // Remove the compilation error.
            return std::nullopt;
        }

        const auto reference = api_.ref(lua_state, lua_registry_index);
        references.emplace(lua_state, reference);
        return reference;
    }
}
