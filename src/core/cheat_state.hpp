#pragma once

#include <atomic>
#include <mutex>
#include <utility>
#include <vector>

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
        std::atomic_bool esp_enabled{false};

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
