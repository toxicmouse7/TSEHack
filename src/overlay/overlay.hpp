#pragma once

namespace core { struct CheatState; }

namespace overlay
{
    bool install(core::CheatState& state);
    void remove();
}
