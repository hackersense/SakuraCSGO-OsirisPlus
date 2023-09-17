#include <algorithm>
#include <array>

#include "Aimbot.h"
#include "Backtrack.h"
#include "../ConfigStructs.h"
#include "../Interfaces.h"
#include "../Memory.h"
#include "../SDK/Cvar.h"
#include "../SDK/ConVar.h"
#include "../SDK/Engine.h"
#include "../SDK/Entity.h"
#include "../SDK/ModelInfo.h"
#include "../SDK/EntityList.h"
#include <SDK/Constants/FrameStage.h>
#include "../SDK/GlobalVars.h"
#include "../SDK/LocalPlayer.h"
#include "../SDK/NetworkChannel.h"
#include "../SDK/UserCmd.h"

struct BacktrackConfig {
    bool enabled = false;
    bool ignoreSmoke = false;
    bool ignoreFlash = false;
    bool recoilBasedFov = false;
    int timeLimit = 200;
    bool fakeLatency = false;
    int fakeLatencyAmount = 200;
} backtrackConfig;

static std::array<std::deque<Record>, 65> records;
static std::deque<InComingSequence> sequences;

struct Cvars {
    ConVar* updateRate;
    ConVar* maxUpdateRate;
    ConVar* interp;
    ConVar* interpRatio;
    ConVar* minInterpRatio;
    ConVar* maxInterpRatio;
    ConVar* maxUnlag;
};

static Cvars cvars;

bool Backtrack::isEnabled() noexcept
{
    return backtrackConfig.enabled;
}

bool Backtrack::fakePing() noexcept
{
    return backtrackConfig.fakeLatency;
}

int Backtrack::pingAmount() noexcept
{
    return backtrackConfig.fakeLatencyAmount;
}

float Backtrack::getLerp() noexcept
{
    auto ratio = std::clamp(cvars.interpRatio->getFloat(), cvars.minInterpRatio->getFloat(), cvars.maxInterpRatio->getFloat());
    return (std::max)(cvars.interp->getFloat(), (ratio / ((cvars.maxUpdateRate) ? cvars.maxUpdateRate->getFloat() : cvars.updateRate->getFloat())));
}

float Backtrack::getExtraTicks() noexcept
{
    if (!backtrackConfig.fakeLatency || backtrackConfig.fakeLatencyAmount <= 0)
        return 0.f;
    return static_cast<float>(backtrackConfig.fakeLatencyAmount) / 1000.f;
}

static auto timeToTicks(float time) noexcept
{
    return static_cast<int>(0.5f + time / memory->globalVars->intervalPerTick);
}

void Backtrack::update(csgo::FrameStage stage) noexcept
{
    if (stage == csgo::FrameStage::RENDER_START) {
        if (!backtrackConfig.enabled || !localPlayer || !localPlayer->isAlive()) {
            for (auto& record : records)
                record.clear();
            return;
        }

        for (int i = 1; i <= interfaces->engine->getMaxClients(); i++) {
            auto entity = interfaces->entityList->getEntity(i);
            if (!entity || entity == localPlayer.get() || entity->isDormant() || !entity->isAlive() || !entity->isOtherEnemy(localPlayer.get())) {
                records[i].clear();
                continue;
            }

            if (!records[i].empty() && (records[i].front().simulationTime == entity->simulationTime()))
                continue;

            Record record{ };
            record.origin = entity->getAbsOrigin();
            record.simulationTime = entity->simulationTime();

            entity->setupBones(record.matrix, 256, BONE_USED_BY_ANYTHING, memory->globalVars->currenttime);

            records[i].push_front(record);

            while (records[i].size() > 3 && records[i].size() > static_cast<size_t>(timeToTicks(static_cast<float>(backtrackConfig.timeLimit) / 1000.f + getExtraTicks())))
                records[i].pop_back();

            //if (auto invalid = std::find_if(std::cbegin(records[i]), std::cend(records[i]), [](const Record& rec) { return !valid(rec.simulationTime); }); invalid != std::cend(records[i]))
            //    records[i].erase(invalid, std::cend(records[i]));
        }
    }
}

void Backtrack::run(UserCmd* cmd) noexcept
{
    if (!backtrackConfig.enabled)
        return;

    if (!(cmd->buttons & UserCmd::IN_ATTACK))
        return;

    if (!localPlayer || !localPlayer->isAlive())
        return;

    if (!backtrackConfig.ignoreFlash && localPlayer->isFlashed())
        return;

    const auto activeWeapon = localPlayer->getActiveWeapon();
    if (!activeWeapon || !activeWeapon->clip())
        return;

    auto localPlayerEyePosition = localPlayer->getEyePosition();

    auto bestFov{ 255.f };
    Entity* bestTarget{ };
    int bestTargetIndex{ };
    Vector bestTargetOrigin{ };
    int bestRecord{ };

    const auto aimPunch = (backtrackConfig.recoilBasedFov && localPlayer->getActiveWeapon()->requiresRecoilControl()) ? localPlayer->getAimPunch() : Vector{ };

    for (int index = 1; index <= interfaces->engine->getMaxClients(); index++) {
        auto entity = interfaces->entityList->getEntity(index);
        if (!entity || entity == localPlayer.get() || entity->isDormant() || !entity->isAlive()
            || !entity->isOtherEnemy(localPlayer.get()))
            continue;

        const auto& origin = entity->getAbsOrigin();

        auto angle = Aimbot::calculateRelativeAngle(localPlayerEyePosition, origin, cmd->viewangles + aimPunch);
        auto fov = std::hypotf(angle.x, angle.y);
        if (fov < bestFov) {
            bestFov = fov;
            bestTarget = entity;
            bestTargetIndex = index;
            bestTargetOrigin = origin;
        }
    }

    if (bestTarget) {
        if (records.empty() || (!backtrackConfig.ignoreSmoke && memory->lineGoesThroughSmoke(localPlayer->getEyePosition(), bestTargetOrigin, 1)))
            return;

        bestFov = 255.f;

        for (size_t i = 0; i < records[bestTargetIndex].size(); i++) {
            const auto& record = records[bestTargetIndex][i];
            if (!valid(record.simulationTime))
                continue;

            auto angle = Aimbot::calculateRelativeAngle(localPlayerEyePosition, record.origin, cmd->viewangles + aimPunch);
            auto fov = std::hypotf(angle.x, angle.y);
            if (fov < bestFov) {
                bestFov = fov;
                bestRecord = i;
            }
        }
    }

    if (bestRecord) {
        const auto& record = records[bestTargetIndex][bestRecord];
        memory->setAbsOrigin(bestTarget, record.origin);
        cmd->tickCount = timeToTicks(record.simulationTime + getLerp());
    }
}

const std::deque<Record>* Backtrack::getRecords(std::size_t index) noexcept
{
    if (!backtrackConfig.enabled)
        return nullptr;
    return &records[index];
}

void Backtrack::addLatencyToNetwork(NetworkChannel* network, float latency) noexcept
{
    for (auto& sequence : sequences)
    {
        if (memory->globalVars->serverTime() - sequence.serverTime >= latency)
        {
            network->inReliableState = sequence.inReliableState;
            network->inSequenceNr = sequence.sequencenr;
            break;
        }
    }
}

void Backtrack::updateIncomingSequences() noexcept
{
    static int lastIncomingSequenceNumber = 0;

    //if (!backtrackConfig.fakeLatency)
    //    return;

    if (!localPlayer)
        return;

    //if (localPlayer->getTeamNumber() == csgo::Team::None)
    //    return;

    const auto network = interfaces->engine->getNetworkChannel();
    if (!network)
        return;

    if (network->inSequenceNr != lastIncomingSequenceNumber)
    {
        lastIncomingSequenceNumber = network->inSequenceNr;

        InComingSequence sequence{ };
        sequence.inReliableState = network->inReliableState;
        sequence.sequencenr = network->inSequenceNr;
        sequence.serverTime = memory->globalVars->serverTime();
        sequences.push_front(sequence);
    }

    while (sequences.size() > 2048)
        sequences.pop_back();
}

bool Backtrack::valid(float simtime) noexcept
{
    const auto network = interfaces->engine->getNetworkChannel();
    if (!network)
        return false;

    auto delta = std::clamp(network->getLatency(0) + network->getLatency(1) + getLerp(), 0.f, cvars.maxUnlag->getFloat()) - (memory->globalVars->serverTime() - simtime);
    return std::abs(delta) <= 0.2f;
}

void Backtrack::init() noexcept
{
    cvars.updateRate = interfaces->cvar->findVar("cl_updaterate");
    cvars.maxUpdateRate = interfaces->cvar->findVar("sv_maxupdaterate");
    cvars.interp = interfaces->cvar->findVar("cl_interp");
    cvars.interpRatio = interfaces->cvar->findVar("cl_interp_ratio");
    cvars.minInterpRatio = interfaces->cvar->findVar("sv_client_min_interp_ratio");
    cvars.maxInterpRatio = interfaces->cvar->findVar("sv_client_max_interp_ratio");
    cvars.maxUnlag = interfaces->cvar->findVar("sv_maxunlag");
}

static bool backtrackWindowOpen = false;

void Backtrack::menuBarItem() noexcept
{
    if (ImGui::MenuItem(Language::getText(LanguageID::GUI_TAB_BACKTRACK))) {
        backtrackWindowOpen = true;
        ImGui::SetWindowFocus(Language::getText(LanguageID::GUI_TAB_BACKTRACK));
        ImGui::SetWindowPos(Language::getText(LanguageID::GUI_TAB_BACKTRACK), { 100.0f, 100.0f });
    }
}

void Backtrack::tabItem() noexcept
{
    if (ImGui::BeginTabItem(Language::getText(LanguageID::GUI_TAB_BACKTRACK))) {
        drawGUI(true);
        ImGui::EndTabItem();
    }
}

void Backtrack::drawGUI(bool contentOnly) noexcept
{
    if (!contentOnly) {
        if (!backtrackWindowOpen)
            return;
        ImGui::SetNextWindowSize({ 0.0f, 0.0f });
        ImGui::Begin(Language::getText(LanguageID::GUI_TAB_BACKTRACK), &backtrackWindowOpen, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    }
    ImGui::Checkbox(Language::getText(LanguageID::GUI_GLOBAL_ENABLED), &backtrackConfig.enabled);
    ImGui::Checkbox("Ignore smoke", &backtrackConfig.ignoreSmoke);
    ImGui::Checkbox("Ignore flash", &backtrackConfig.ignoreFlash);
    ImGui::Checkbox("Recoil based fov", &backtrackConfig.recoilBasedFov);
    ImGui::PushItemWidth(220.0f);
    ImGui::SliderInt("Time limit", &backtrackConfig.timeLimit, 1, 200, "%d ms");
    ImGui::PopItemWidth();
    ImGui::NextColumn();
    ImGui::Checkbox("Enabled Fake Latency", &backtrackConfig.fakeLatency);
    ImGui::PushItemWidth(220.0f);
    ImGui::SliderInt("Ping", &backtrackConfig.fakeLatencyAmount, 1, 1000, "%d ms");
    ImGui::PopItemWidth();
    ImGui::Columns(1);
    if (!contentOnly)
        ImGui::End();
}

static void to_json(json& j, const BacktrackConfig& o, const BacktrackConfig& dummy = {})
{
    WRITE("Enabled", enabled);
    WRITE("Ignore smoke", ignoreSmoke);
    WRITE("Ignore flash", ignoreFlash);
    WRITE("Recoil based fov", recoilBasedFov);
    WRITE("Time limit", timeLimit);
    WRITE("Fake Latency", fakeLatency);
    WRITE("Fake Latency Amount", fakeLatencyAmount);
}

json Backtrack::toJson() noexcept
{
    json j;
    to_json(j, backtrackConfig);
    return j;
}

static void from_json(const json& j, BacktrackConfig& b)
{
    read(j, "Enabled", b.enabled);
    read(j, "Ignore smoke", b.ignoreSmoke);
    read(j, "Ignore flash", b.ignoreFlash);
    read(j, "Recoil based fov", b.recoilBasedFov);
    read(j, "Time limit", b.timeLimit);
    read(j, "Fake Latency", b.fakeLatency);
    read(j, "Fake Latency Amount", b.fakeLatencyAmount);
}

void Backtrack::fromJson(const json& j) noexcept
{
    from_json(j, backtrackConfig);
}

void Backtrack::resetConfig() noexcept
{
    backtrackConfig = {};
}