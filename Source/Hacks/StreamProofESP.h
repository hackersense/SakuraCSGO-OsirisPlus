#pragma once

#include <Config.h>
#include "../Memory.h"

namespace StreamProofESP
{
    void render() noexcept;
    void updateInput() noexcept;

    // GUI
    void menuBarItem() noexcept;
    void tabItem() noexcept;
    void drawGUI(bool contentOnly) noexcept;
}
