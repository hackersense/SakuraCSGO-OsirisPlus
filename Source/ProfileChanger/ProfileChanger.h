#pragma once

#include "../JsonForward.h"

namespace ProfileChanger
{
    bool isEnabled() noexcept;
    int getFriendly() noexcept;
    int getTeacher() noexcept;
    int getLeader() noexcept;
    int getRank() noexcept;
    int getT_Rank() noexcept;
    int getR_Rank() noexcept;
    int getEXP() noexcept;
    int getLevel() noexcept;
    int getWins() noexcept;
    int getT_Wins() noexcept;
    int getR_Wins() noexcept;
    int getBanType() noexcept;
    int getBanTime() noexcept;
    void onUpdateAll() noexcept;

    // GUI
    void menuBarItem() noexcept;
    void tabItem() noexcept;
    void drawGUI(bool contentOnly) noexcept;

    // Config
    json toJson() noexcept;
    void fromJson(const json& j) noexcept;
    void resetConfig() noexcept;
}