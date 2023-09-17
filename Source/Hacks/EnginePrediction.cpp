#include <memory>

#include "../Interfaces.h"
#include "../Memory.h"

#include "../SDK/Entity.h"
#include "../SDK/UserCmd.h"
#include "../SDK/GameMovement.h"
#include "../SDK/GlobalVars.h"
#include "../SDK/LocalPlayer.h"
#include "../SDK/MoveHelper.h"
#include "../SDK/Prediction.h"
#include "../SDK/ClientState.h"

#include "../Utils/MathUtils.h"
#include "../Utils/CSGOUtils.h"

#include "EnginePrediction.h"

static int localPlayerFlags;
static Vector localPlayerVelocity;
static float previousCurrenttime{ 0.0f };
static float previousFrametime{ 0.0f };
static float previousIsFirstTimePredicted{ 0.0f };
static float previousInPrediction{ 0.0f };
static bool inPrediction{ false };
static std::array<EnginePrediction::NetvarData, 150> netvarData;

void EnginePrediction::update() noexcept
{
    if (!localPlayer || !localPlayer->isAlive())
        return;

    const auto deltaTick = CSGOUtils::getClientState()->deltaTick;
    const auto start = CSGOUtils::getClientState()->lastCommandAttack;
    const auto stop = CSGOUtils::getClientState()->lastOutgoingCommand + CSGOUtils::getClientState()->chokedCommands;
    interfaces->prediction->update(deltaTick, deltaTick > 0, start, stop);
}

void EnginePrediction::start(UserCmd* cmd) noexcept
{
    if (!localPlayer || !localPlayer->isAlive())
        return;

    localPlayerFlags = localPlayer->flags();
    localPlayerVelocity = localPlayer->velocity();

    inPrediction = true;

    *memory->predictionRandomSeed = MathUtils::MD5_PseudoRandom(cmd->commandNumber) & INT_MAX; // 0x7FFFFFFF
    *memory->predictionPlayer = reinterpret_cast<int>(localPlayer.get());

    previousCurrenttime = memory->globalVars->currenttime;
    previousFrametime = memory->globalVars->frametime;
    previousIsFirstTimePredicted = interfaces->prediction->isFirstTimePredicted;
    previousInPrediction = interfaces->prediction->inPrediction;

    memory->globalVars->currenttime = memory->globalVars->serverTime();
    memory->globalVars->frametime = memory->globalVars->intervalPerTick;
    interfaces->prediction->isFirstTimePredicted = false;
    interfaces->prediction->inPrediction = true;

    if (cmd->impulse)
        *reinterpret_cast<uint32_t*>(reinterpret_cast<uintptr_t>(localPlayer.get()) + 0x320C) = cmd->impulse;

    cmd->buttons |= *reinterpret_cast<int*>(reinterpret_cast<uintptr_t>(localPlayer.get()) + 0x3344);
    cmd->buttons &= ~(*reinterpret_cast<int*>(reinterpret_cast<uintptr_t>(localPlayer.get()) + 0x3340));

    localPlayer->updateButtonState(cmd->buttons);

    interfaces->prediction->checkMovingGround(localPlayer.get(), memory->globalVars->frametime);
    memory->moveHelper->setHost(localPlayer.get());
    interfaces->gameMovement->startTrackPredictionErrors(localPlayer.get());

    interfaces->prediction->setupMove(localPlayer.get(), cmd, memory->moveHelper, memory->moveData);
    interfaces->gameMovement->processMovement(localPlayer.get(), memory->moveData);
    interfaces->prediction->finishMove(localPlayer.get(), cmd, memory->moveData);
    memory->moveHelper->processImpacts();

    const auto activeWeapon = localPlayer->getActiveWeapon();
    if (!activeWeapon || activeWeapon->isGrenade() || activeWeapon->isKnife())
        return;

    activeWeapon->updateAccuracyPenalty();
}

void EnginePrediction::finsh() noexcept
{
    interfaces->gameMovement->finishTrackPredictionErrors(localPlayer.get());
    memory->moveHelper->setHost(nullptr);
    interfaces->gameMovement->reset();

    *memory->predictionRandomSeed = -1;
    *memory->predictionPlayer = 0;

    memory->globalVars->currenttime = previousCurrenttime;
    memory->globalVars->frametime = previousFrametime;
    interfaces->prediction->isFirstTimePredicted = previousIsFirstTimePredicted;
    interfaces->prediction->inPrediction = previousInPrediction;

    inPrediction = false;
}

void EnginePrediction::store() noexcept
{
    if (!localPlayer || !localPlayer->isAlive())
        return;

    const int tickbase = localPlayer->tickBase();

    NetvarData netvars{ };

    netvars.tickbase = tickbase;

    netvars.aimPunchAngle = localPlayer->aimPunchAngle();
    netvars.aimPunchAngleVelocity = localPlayer->aimPunchAngleVelocity();
    netvars.baseVelocity = localPlayer->baseVelocity();
    netvars.duckAmount = localPlayer->duckAmount();
    netvars.duckSpeed = localPlayer->duckSpeed();
    netvars.fallVelocity = localPlayer->fallVelocity();
    netvars.thirdPersonRecoil = localPlayer->thirdPersonRecoil();
    netvars.velocity = localPlayer->velocity();
    netvars.velocityModifier = localPlayer->velocityModifier();
    netvars.viewPunchAngle = localPlayer->viewPunchAngle();
    netvars.viewOffset = localPlayer->viewOffset();

    netvarData.at(tickbase % 150) = netvars;
}

void EnginePrediction::apply(csgo::FrameStage stage) noexcept
{
    if (stage != csgo::FrameStage::NET_UPDATE_END)
        return;

    if (!localPlayer || !localPlayer->isAlive())
        return;

    if (netvarData.empty())
        return;

    const int tickbase = localPlayer->tickBase();

    const auto netvars = netvarData.at(tickbase % 150);

    if (!&netvars)
        return;

    if (netvars.tickbase != tickbase)
        return;

    localPlayer->aimPunchAngle() = NetvarData::checkDifference(localPlayer->aimPunchAngle(), netvars.aimPunchAngle);
    localPlayer->aimPunchAngleVelocity() = NetvarData::checkDifference(localPlayer->aimPunchAngleVelocity(), netvars.aimPunchAngleVelocity);
    localPlayer->baseVelocity() = NetvarData::checkDifference(localPlayer->baseVelocity(), netvars.baseVelocity);
    localPlayer->duckAmount() = std::clamp(NetvarData::checkDifference(localPlayer->duckAmount(), netvars.duckAmount), 0.0f, 1.0f);
    localPlayer->duckSpeed() = NetvarData::checkDifference(localPlayer->duckSpeed(), netvars.duckSpeed);
    localPlayer->fallVelocity() = NetvarData::checkDifference(localPlayer->fallVelocity(), netvars.fallVelocity);
    localPlayer->thirdPersonRecoil() = NetvarData::checkDifference(localPlayer->thirdPersonRecoil(), netvars.thirdPersonRecoil);
    localPlayer->velocity() = NetvarData::checkDifference(localPlayer->velocity(), netvars.velocity);
    localPlayer->velocityModifier() = NetvarData::checkDifference(localPlayer->velocityModifier(), netvars.velocityModifier);
    localPlayer->viewPunchAngle() = NetvarData::checkDifference(localPlayer->viewPunchAngle(), netvars.viewPunchAngle);
    localPlayer->viewOffset() = NetvarData::checkDifference(localPlayer->viewOffset(), netvars.viewOffset);
}

int EnginePrediction::getFlags() noexcept
{
    return localPlayerFlags;
}

Vector EnginePrediction::getVelocity() noexcept
{
    return localPlayerVelocity;
}

bool EnginePrediction::isInPrediction() noexcept
{
    return inPrediction;
}