#pragma once

#include <cstddef>
#include <deque>

#include "../JsonForward.h"

#include "../SDK/Matrix3x4.h"
#include "../SDK/Vector.h"

#include "../Interfaces.h"
#include "../Memory.h"

namespace csgo { enum class FrameStage; }
class NetworkChannel;
struct UserCmd;

struct Record
{
    Vector origin;
    float simulationTime;
    matrix3x4 matrix[256];
};

struct InComingSequence {
    int inReliableState;
    int sequencenr;
    float serverTime;
};

namespace Backtrack
{
    void update(csgo::FrameStage) noexcept;
    void run(UserCmd*) noexcept;

    const std::deque<Record>* getRecords(std::size_t index) noexcept;
    void addLatencyToNetwork(NetworkChannel* network, float latency) noexcept;
    void updateIncomingSequences() noexcept;
    bool valid(float simtime) noexcept;
    void init() noexcept;
    bool isEnabled() noexcept;
    bool fakePing() noexcept;
    int pingAmount() noexcept;
    float getLerp() noexcept;
    float getExtraTicks() noexcept;

    // GUI
    void menuBarItem() noexcept;
    void tabItem() noexcept;
    void drawGUI(bool contentOnly) noexcept;

    // Config
    json toJson() noexcept;
    void fromJson(const json& j) noexcept;
    void resetConfig() noexcept;
}
