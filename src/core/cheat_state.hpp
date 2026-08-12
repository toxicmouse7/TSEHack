#pragma once

#include <atomic>
#include <mutex>
#include <utility>
#include <vector>
#include <windows.h>

namespace core
{
    struct EspBox
    {
        float left;
        float top;
        float right;
        float bottom;
    };

    // Shared application state. It deliberately contains no hook, renderer, or game details.
    struct CheatState
    {
        std::atomic_bool target_assist_enabled{false};
        std::atomic_bool target_assist_require_activation_key{true};
        std::atomic_int target_assist_activation_key{VK_RBUTTON};
        std::atomic_bool esp_enabled{false};
        std::atomic_bool unlock_requested{false};
        std::atomic_bool unlock_installed{false};

        void set_esp_boxes(std::vector<EspBox> boxes)
        {
            std::lock_guard lock(esp_boxes_mutex_);
            esp_boxes_ = std::move(boxes);
        }

        [[nodiscard]] std::vector<EspBox> esp_boxes() const
        {
            std::lock_guard lock(esp_boxes_mutex_);
            return esp_boxes_;
        }

    private:
        mutable std::mutex esp_boxes_mutex_;
        std::vector<EspBox> esp_boxes_;
    };
}
