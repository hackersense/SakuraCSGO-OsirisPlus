#include <imgui/imgui.h>
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui/imgui_internal.h>
#include <imgui/imgui_stdlib.h>

#include "../ConfigStructs.h"
#include "ProfileChanger.h"
#include "Protobuffs.h"

static const char* bans[] =
{
    "Off",
    "You were kicked from the last match (Competitive cooldown)",
    "You killed too many teammates (Competitive cooldown)",
    "You killed a teammate at round start (Competitive cooldown)",
    "You failed to reconnect to the last match (Competitive cooldown)",
    "You abandoned the last match (Competitive cooldown)",
    "You did too much damage to your teammates (Competitive cooldown)",
    "You did too much damage to your teammates at round start (Competitive cooldown)",
    "This account is permanently untrusted (Global cooldown)",
    "You were kicked from too many recent matches (Competitive cooldown)",
    "Convicted by overwatch - majorly disruptive (Global cooldown)",
    "Convicted by overwatch - minorly disruptive (Global cooldown)",
    "Resolving matchmaking state for your account (Temporary cooldown)",
    "Resolving matchmaking state after the last match (Temporary cooldown)",
    "This account is permanently untrusted (Global cooldown)",
    "(Global cooldown)",
    "You failed to connect by match start. (Competitive cooldown)",
    "You have kicked too many teammates in recent matches (Competitive cooldown)",
    "Congratulations on your recent competitive wins! before you play competitive matches further please wait for matchmaking servers to calibrate your skill group placement based on your lastest performance. (Temporary cooldown)",
    "A server using your game server login token has been banned. your account is now permanently banned from operating game servers, and you have a cooldown from connecting to game servers. (Global cooldown)"
};

struct ProfileChangerConfig {
    bool enabled = false;
    int friendly = 0;
    int teacher = 0;
    int leader = 0;
    int rank = 0;
    int wins = 0;
    int r_rank = 0;
    int r_wins = 0;
    int t_rank = 0;
    int t_wins = 0;
    int level = 0;
    int exp = 0;
    int ban_type = 0;
    int ban_time = 0;
} profileChangerConfig;

static void from_json(const json& j, ProfileChangerConfig& p)
{
    read(j, "Enabled", p.enabled);
    read(j, "Friendly", p.friendly);
    read(j, "Teacher", p.teacher);
    read(j, "Leader", p.leader);
    read(j, "Rank", p.rank);
    read(j, "Wins", p.wins);
    read(j, "R_Rank", p.r_rank);
    read(j, "R_Wins", p.r_wins);
    read(j, "T_Rank", p.t_rank);
    read(j, "T_Wins", p.t_wins);
    read(j, "Level", p.level);
    read(j, "Exp", p.exp);
}

static void to_json(json& j, const ProfileChangerConfig& o)
{
    const ProfileChangerConfig dummy;

    WRITE("Enabled", enabled);
    WRITE("Friendly", friendly);
    WRITE("Teacher", teacher);
    WRITE("Leader", leader);
    WRITE("Rank", rank);
    WRITE("Wins", wins);
    WRITE("R_Rank", r_rank);
    WRITE("R_Wins", r_wins);
    WRITE("T_Rank", t_rank);
    WRITE("T_Wins", t_wins);
    WRITE("Level", level);
    WRITE("Exp", exp);
}

bool ProfileChanger::isEnabled() noexcept
{
    return profileChangerConfig.enabled;
}

int ProfileChanger::getFriendly() noexcept
{
    return profileChangerConfig.friendly;
}

int ProfileChanger::getTeacher() noexcept
{
    return profileChangerConfig.teacher;
}

int ProfileChanger::getLeader() noexcept
{
    return profileChangerConfig.leader;
}

int ProfileChanger::getRank() noexcept
{
    return profileChangerConfig.rank;
}

int ProfileChanger::getT_Rank() noexcept
{
    return profileChangerConfig.t_rank;
}

int ProfileChanger::getR_Rank() noexcept
{
    return profileChangerConfig.r_rank;
}

int ProfileChanger::getEXP() noexcept
{
    return profileChangerConfig.exp;
}

int ProfileChanger::getLevel() noexcept
{
    return profileChangerConfig.level;
}

int ProfileChanger::getWins() noexcept
{
    return profileChangerConfig.wins;
}

int ProfileChanger::getT_Wins() noexcept
{
    return profileChangerConfig.t_wins;
}

int ProfileChanger::getR_Wins() noexcept
{
    return profileChangerConfig.r_wins;
}

int ProfileChanger::getBanType() noexcept
{
    return profileChangerConfig.ban_type;
}

int ProfileChanger::getBanTime() noexcept
{
    return profileChangerConfig.ban_time;
}

void ProfileChanger::onUpdateAll() noexcept
{
    write.sendClientHelloFix();
    write.sendClientGcRankUpdateMatchmaking();
    write.sendClientGcRankUpdateWingman();
    write.sendClientGcRankUpdateZone();
}

static bool windowOpen = false;

void ProfileChanger::menuBarItem() noexcept
{
    if (ImGui::MenuItem(Language::getText(LanguageID::GUI_TAB_PROFILECHANGER))) {
        windowOpen = true;
        ImGui::SetWindowFocus(Language::getText(LanguageID::GUI_TAB_PROFILECHANGER));
        ImGui::SetWindowPos(Language::getText(LanguageID::GUI_TAB_PROFILECHANGER), { 100.0f, 100.0f });
    }
}

void ProfileChanger::tabItem() noexcept
{
    if (ImGui::BeginTabItem(Language::getText(LanguageID::GUI_TAB_PROFILECHANGER))) {
        drawGUI(true);
        ImGui::EndTabItem();
    }
}

void ProfileChanger::drawGUI(bool contentOnly) noexcept
{
    if (!contentOnly) {
        if (!windowOpen)
            return;
        ImGui::SetNextWindowSize({ 0.0f, 0.0f });
        ImGui::Begin(Language::getText(LanguageID::GUI_TAB_PROFILECHANGER), &windowOpen, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize
            | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    }

    const char* mmrank = "Off\0Silver 1\0Silver 2\0Silver 3\0Silver 4\0Silver elite\0Silver elite master\0Gold nova 1\0Gold nova 2\0Gold nova 3\0Gold nova master\0Master guardian 1\0Master guardian 1\0Master guardian elite\0Distinguished master guardian\0Legendary eagle\0Legendary eagle master\0Supreme master first class\0The global elite\0";
    const char* dzrank = "Off\0Lab rat 1\0Lab rat 2\0Sprinting hare 1\0Sprinting hare 2\0Wild scout 1\0Wild scout elite\0Hunter fox 1\0Hunter fox 2\0Hunter fox 3\0Hunter fox elite\0Timber wolf\0Ember wolf\0Wildfire wolf\0The howling alpha\0";

    ImGui::Checkbox("Enabled", &profileChangerConfig.enabled);
    ImGui::Combo("Rank", &profileChangerConfig.rank, mmrank);
    ImGui::InputInt("Wins", &profileChangerConfig.wins);
    ImGui::Combo("Wingman Rank", &profileChangerConfig.r_rank, mmrank);
    ImGui::InputInt("Wingman Wins", &profileChangerConfig.r_wins);
    ImGui::Combo("DangerZone Rank", &profileChangerConfig.t_rank, dzrank);
    ImGui::InputInt("DangerZone Wins", &profileChangerConfig.t_wins);
    ImGui::SliderInt("Level", &profileChangerConfig.level, 0, 40);
    ImGui::InputInt("EXP", &profileChangerConfig.exp);
    ImGui::InputInt("Friendly", &profileChangerConfig.friendly);
    ImGui::InputInt("Teacher", &profileChangerConfig.teacher);
    ImGui::InputInt("Leader", &profileChangerConfig.leader);
    ImGui::Combo("Fake ban", &profileChangerConfig.ban_type, bans, IM_ARRAYSIZE(bans));
    ImGui::SliderInt("Fake ban time", &profileChangerConfig.ban_time, 0, 1000, "Seconds: %d");

    if (ImGui::Button("ApplyMatchMaking", ImVec2(190, 30)) && profileChangerConfig.enabled)
    {
        write.sendClientHelloFix();
        write.sendClientGcRankUpdateMatchmaking();
    }

    if (ImGui::Button("ApplyWingman", ImVec2(190, 30)) && profileChangerConfig.enabled)
    {
        write.sendClientHelloFix();
        write.sendClientGcRankUpdateWingman();
    }

    if (ImGui::Button("ApplyZone", ImVec2(190, 30)) && profileChangerConfig.enabled)
    {
        write.sendClientHelloFix();
        write.sendClientGcRankUpdateZone();
    }

    if (!contentOnly)
        ImGui::End();
}

json ProfileChanger::toJson() noexcept
{
    json j;
    to_json(j, profileChangerConfig);
    return j;
}

void ProfileChanger::fromJson(const json& j) noexcept
{
    from_json(j, profileChangerConfig);
}

void ProfileChanger::resetConfig() noexcept
{
    profileChangerConfig = {};
}
