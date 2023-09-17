#include <charconv>
#include <functional>
#include <string>

#include "imgui/imgui.h"

#ifdef _WIN32
#include <intrin.h>
#include <Windows.h>
#include <Psapi.h>

#include "imgui/imgui_impl_dx9.h"
#include "imgui/imgui_impl_win32.h"

#include "MinHook/MinHook.h"
#elif __linux__
#include <sys/mman.h>
#include <unistd.h>

#include <SDL2/SDL.h>

#include "imgui/imgui_impl_sdl.h"
#include "imgui/imgui_impl_opengl3.h"
#endif

#include "Config.h"
#include "EventListener.h"
#include "GameData.h"
#include "GUI.h"
#include "Hooks.h"
#include "Interfaces.h"
#include "Memory.h"

#include "Hacks/Aimbot.h"
#include "Hacks/AntiAim.h"
#include "Hacks/Backtrack.h"
#include "Hacks/Chams.h"
#include "Hacks/EnginePrediction.h"
#include "Hacks/GrenadePrediction.h"
#include "Hacks/StreamProofESP.h"
#include "Hacks/Glow.h"
#include "Hacks/Misc.h"
#include "Hacks/Sound.h"
#include "Hacks/Triggerbot.h"
#include "Hacks/Visuals.h"
#include "Hacks/RadarHack/WebRadar.h"

#include "ProfileChanger/Protobuffs.h"
#include "ProfileChanger/ProfileChanger.h"
#include "InventoryChanger/InventoryChanger.h"

#include "SDK/ClientClass.h"
#include "SDK/Cvar.h"
#include "SDK/ConVar.h"
#include "SDK/Engine.h"
#include "SDK/Entity.h"
#include "SDK/EntityList.h"
#include "SDK/Constants/FrameStage.h"
#include "SDK/GameEvent.h"
#include "SDK/GameUI.h"
#include "SDK/Input.h"
#include "SDK/GlobalVars.h"
#include "SDK/InputSystem.h"
#include "SDK/ItemSchema.h"
#include "SDK/Prediction.h"
#include "SDK/LocalPlayer.h"
#include "SDK/MaterialSystem.h"
#include "SDK/ModelRender.h"
#include "SDK/Platform.h"
#include "SDK/RenderContext.h"
#include "SDK/SoundInfo.h"
#include "SDK/SoundEmitter.h"
#include "SDK/StudioRender.h"
#include "SDK/Surface.h"
#include "SDK/UserCmd.h"
#include "SDK/SteamAPI.h"
#include "SDK/ViewSetup.h"
#include "SDK/UtlRbTree.h"
#include "SDK/CStudioHdr.h"
#include "SDK/SplitScreen.h"
#include "SDK/NetworkChannel.h"
#include "SDK/ParticleCollection.h"
#include "SDK/Constants/UserMessages.h"

#include "Utils/AntiDetection.h"
#include "Utils/WindowUtils.h"
#include "Utils/CSGOUtils.h"

#ifdef _WIN32

LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

static LRESULT __stdcall wndProc(HWND window, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
{
    [[maybe_unused]] static const auto once = [](HWND window) noexcept {
        Netvars::init();
        EventListener::init();

        ImGui::CreateContext();
        ImGui_ImplWin32_Init(window);
        config.emplace(Config{});
        gui.emplace(GUI{});

        hooks->install();

        return true;
    }(window);

    ImGui_ImplWin32_WndProcHandler(window, msg, wParam, lParam);
    //interfaces->inputSystem->enableInput(!gui->isOpen());
    interfaces->inputSystem->enableInput(!(gui->isOpen() && ((wParam != 'W' && wParam != 'A' && wParam != 'S' && wParam != 'D' && wParam != VK_SHIFT && wParam != VK_CONTROL && wParam != VK_SPACE) || ImGui::GetIO().WantTextInput)));

    return CallWindowProcW(hooks->originalWndProc, window, msg, wParam, lParam);
}

static HRESULT __stdcall reset(IDirect3DDevice9* device, D3DPRESENT_PARAMETERS* params) noexcept
{
    ImGui_ImplDX9_InvalidateDeviceObjects();
    InventoryChanger::clearItemIconTextures();
    GameData::clearTextures();
    return hooks->originalReset(device, params);
}

#endif

#ifdef _WIN32
static HRESULT __stdcall present(IDirect3DDevice9* device, const RECT* src, const RECT* dest, HWND windowOverride, const RGNDATA* dirtyRegion) noexcept
{
    [[maybe_unused]] static bool imguiInit{ ImGui_ImplDX9_Init(device) };

    if (config->loadScheduledFonts())
        ImGui_ImplDX9_DestroyFontsTexture();

    ImGui_ImplDX9_NewFrame();
    ImGui_ImplWin32_NewFrame();
#else
static void swapWindow(SDL_Window * window) noexcept
{
    [[maybe_unused]] static const auto _ = ImGui_ImplSDL2_InitForOpenGL(window, nullptr);

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame(window);
#endif
    ImGui::NewFrame();

    if (const auto& displaySize = ImGui::GetIO().DisplaySize; displaySize.x > 0.0f && displaySize.y > 0.0f) {
        StreamProofESP::render();
        GrenadePrediction::render();
        Misc::purchaseList();
        Misc::noscopeCrosshair(ImGui::GetBackgroundDrawList());
        Misc::recoilCrosshair(ImGui::GetBackgroundDrawList());
        Misc::drawOffscreenEnemies(ImGui::GetBackgroundDrawList());
        Misc::drawBombTimer();
        Misc::spectatorList();
        Visuals::hitMarker(nullptr, ImGui::GetBackgroundDrawList());
        Visuals::drawMolotovHull(ImGui::GetBackgroundDrawList());
        Misc::watermark();

        Aimbot::updateInput();
        Visuals::updateInput();
        StreamProofESP::updateInput();
        Misc::updateInput();
        Triggerbot::updateInput();
        Chams::updateInput();
        Glow::updateInput();

        gui->handleToggle();

        if (gui->isOpen())
            gui->render();
    }

    ImGui::EndFrame();
    ImGui::Render();

#ifdef _WIN32
    if (device->BeginScene() == D3D_OK) {
        ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
        device->EndScene();
    }
#else
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
#endif

    GameData::clearUnusedAvatars();
    InventoryChanger::clearUnusedItemIconTextures();

#ifdef _WIN32
    return hooks->originalPresent(device, src, dest, windowOverride, dirtyRegion);
#else
    hooks->swapWindow(window);
#endif
}

#ifndef _WIN32
static bool STDCALL_CONV createMove(LINUX_ARGS(void* thisptr,) float inputSampleTime, UserCmd* cmd) noexcept
{
    auto result = hooks->clientMode.callOriginal<bool, WIN32_LINUX(24, 25)>(inputSampleTime, cmd);

    if (!cmd->commandNumber)
        return result;

    bool dummy;
    bool& sendPacket = dummy;
#else
static bool STDCALL_CONV createMove(float inputSampleTime, UserCmd* cmd, bool& sendPacket) noexcept
{
#endif
    static auto previousViewAngles{ cmd->viewangles };
    auto currentViewAngles{ cmd->viewangles };
    const auto currentPredictedTick{ interfaces->prediction->split->commandsPredicted - 1 };

    memory->globalVars->serverTime(cmd);
    Misc::antiAfkKick(cmd);
    Misc::fastStop(cmd);
    Misc::prepareRevolver(cmd);
    Visuals::removeShadows();
    Misc::runReportbot();
    Misc::bunnyHop(cmd);
    Misc::removeCrouchCooldown(cmd);
    Misc::autoPistol(cmd);
    Misc::autoReload(cmd);
    Misc::updateClanTag();
    Misc::fakeBan();
    Misc::stealNames();
    Misc::revealRanks(cmd);
    Misc::quickReload(cmd);
    Misc::fixTabletSignal();
    Misc::slowwalk(cmd);
    WebRadar::run();

#ifdef _WIN32
    Backtrack::updateIncomingSequences();
#endif

    EnginePrediction::update();
    EnginePrediction::start(cmd);
    GrenadePrediction::run(cmd);

    Aimbot::run(cmd);
    Triggerbot::run(cmd);
    Backtrack::run(cmd);
    Misc::miniJump(cmd);
    Misc::edgejump(cmd);
    Misc::moonwalk(cmd);
    Misc::fastPlant(cmd);

    if (!(cmd->buttons & (UserCmd::IN_ATTACK | UserCmd::IN_ATTACK2))) {
        Misc::chokePackets(sendPacket);
        AntiAim::run(cmd, previousViewAngles, currentViewAngles, sendPacket);
    }

    Aimbot::fixMouseDelta(cmd);

    Misc::autoStrafe(cmd, currentViewAngles);
    Misc::jumpBug(cmd);

    EnginePrediction::finsh();

    auto viewAnglesDelta{ cmd->viewangles - previousViewAngles };
    viewAnglesDelta.normalize();
    viewAnglesDelta.x = std::clamp(viewAnglesDelta.x, -Misc::maxAngleDelta(), Misc::maxAngleDelta());
    viewAnglesDelta.y = std::clamp(viewAnglesDelta.y, -Misc::maxAngleDelta(), Misc::maxAngleDelta());

    cmd->viewangles = previousViewAngles + viewAnglesDelta;

    cmd->viewangles.normalize();
    Misc::fixMovement(cmd, currentViewAngles.y);

    cmd->viewangles.x = std::clamp(cmd->viewangles.x, -89.0f, 89.0f);
    cmd->viewangles.y = std::clamp(cmd->viewangles.y, -180.0f, 180.0f);
    cmd->viewangles.z = 0.0f;
    cmd->forwardmove = std::clamp(cmd->forwardmove, -450.0f, 450.0f);
    cmd->sidemove = std::clamp(cmd->sidemove, -450.0f, 450.0f);
    cmd->upmove = std::clamp(cmd->upmove, -320.0f, 320.0f);

    cmd->viewanglesBackup.x = cmd->viewangles.x;
    cmd->viewanglesBackup.y = cmd->viewangles.y;
    cmd->viewanglesBackup.z = cmd->viewangles.z;

    cmd->buttonsBackup = cmd->buttons;

    previousViewAngles = cmd->viewangles;

#ifdef _WIN32
    if (localPlayer && localPlayer->isAlive())
        memory->restoreEntityToPredictedFrame(0, currentPredictedTick);
#endif

    return false;
}

#ifdef _WIN32

static void __stdcall CHLCreateMove(int sequenceNumber, float inputSampleTime, bool active, bool& sendPacket) noexcept
{
    auto result = hooks->client.callOriginal<bool, 22>(sequenceNumber, inputSampleTime, active);

    UserCmd* cmd = memory->input->getUserCmd(sequenceNumber);
    if (!cmd || !cmd->commandNumber)
        return;

    VerifiedUserCmd* verified = memory->input->getVerifiedUserCmd(sequenceNumber);
    if (!verified)
        return;

    bool createmove_result = createMove(inputSampleTime, cmd, sendPacket);

    verified->cmd = *cmd;
    verified->crc = cmd->getChecksum();
}

#pragma warning(disable : 4409)
__declspec(naked) void __stdcall createMoveProxy(int sequenceNumber, float inputSampleTime, bool active)
{
    __asm
    {
        PUSH	EBP
        MOV		EBP, ESP
        PUSH	EBX
        LEA		ECX, [ESP]
        PUSH	ECX
        PUSH	active
        PUSH	inputSampleTime
        PUSH	sequenceNumber
        CALL	CHLCreateMove
        POP		EBX
        POP		EBP
        RETN	0xC
    }
}

#endif

static void STDCALL_CONV doPostScreenEffects(LINUX_ARGS(void* thisptr,) void* param) noexcept
{
    if (interfaces->engine->isInGame()) {
        Visuals::thirdperson();
        Visuals::inverseRagdollGravity();
        Visuals::reduceFlashEffect();
        Visuals::updateBrightness();
        Visuals::remove3dSky();
        Glow::render();
    }
    hooks->clientMode.callOriginal<void, WIN32_LINUX(44, 45)>(param);
}

static float STDCALL_CONV getViewModelFov(LINUX_ARGS(void* thisptr)) noexcept
{
    float additionalFov = Visuals::viewModelFov();
    if (localPlayer) {
        if (const auto activeWeapon = localPlayer->getActiveWeapon(); activeWeapon && activeWeapon->getClientClass()->classId == ClassId::Tablet)
            additionalFov = 0.0f;
    }

    return hooks->clientMode.callOriginal<float, WIN32_LINUX(35, 36)>() + additionalFov;
}

static void STDCALL_CONV drawModelExecute(LINUX_ARGS(void* thisptr,) void* ctx, void* state, const ModelRenderInfo& info, matrix3x4* customBoneToWorld) noexcept
{
    if (interfaces->studioRender->isForcedMaterialOverride())
        return hooks->modelRender.callOriginal<void, 21>(ctx, state, std::cref(info), customBoneToWorld);

    if (Visuals::removeHands(info.model->name) || Visuals::removeSleeves(info.model->name) || Visuals::removeWeapons(info.model->name))
        return;

    if (static Chams chams; !chams.render(ctx, state, info, customBoneToWorld))
        hooks->modelRender.callOriginal<void, 21>(ctx, state, std::cref(info), customBoneToWorld);

    interfaces->studioRender->forcedMaterialOverride(nullptr);
}

static bool FASTCALL_CONV svCheatsGetBool(void* thisptr) noexcept
{
    if (RETURN_ADDRESS() == memory->cameraThink && Visuals::isThirdpersonOn())
        return true;

    return hooks->svCheats.callOriginal<bool, void*, WIN32_LINUX(13, 16)>(thisptr);
}

static void STDCALL_CONV frameStageNotify(LINUX_ARGS(void* thisptr,) csgo::FrameStage stage) noexcept
{
    [[maybe_unused]] static auto backtrackInit = (Backtrack::init(), false);

    if (interfaces->engine->isConnected() && !interfaces->engine->isInGame())
        Misc::changeName(true, nullptr, 0.0f);

    if (stage == csgo::FrameStage::START)
        GameData::update();

    if (stage == csgo::FrameStage::RENDER_START) {
        Misc::preserveKillfeed();
        Misc::disablePanoramablur();
        Visuals::colorWorld();
        Visuals::doPostProcessing();
        Misc::fakePrime();
        Misc::updateEventListeners();
        Visuals::updateEventListeners();
    }
    if (interfaces->engine->isInGame()) {
        Visuals::skybox(stage);
        Visuals::removeBlur(stage);
        Misc::oppositeHandKnife(stage);
        Visuals::removeGrass(stage);
        Visuals::modifySmoke(stage);
        Visuals::disablePostProcessing(stage);
        Visuals::removeVisualRecoil(stage);
        Visuals::applyZoom(stage);
        Misc::fixAnimationLOD(stage);
        Backtrack::update(stage);
    }
    inventory_changer::InventoryChanger::instance().run(stage);

    hooks->client.callOriginal<void, 37>(stage);

#ifdef _WIN32
    if (interfaces->engine->isInGame())
        EnginePrediction::apply(stage);
#endif
}

static int STDCALL_CONV emitSound(LINUX_ARGS(void* thisptr,) SoundParams params) noexcept
{
    if (EnginePrediction::isInPrediction())
        return 0;

    Sound::modulateSound(params.soundEntry, params.entityIndex, params.volume);
    Misc::autoAccept(params.soundEntry);

    params.volume = std::clamp(params.volume, 0.0f, 1.0f);
    return hooks->sound.callOriginal<int, WIN32_LINUX(5, 6)>(params);
}

static bool STDCALL_CONV shouldDrawFog(LINUX_ARGS(void* thisptr)) noexcept
{
#ifdef _WIN32
    if constexpr (std::is_same_v<HookType, MinHook>) {
        if (RETURN_ADDRESS() != memory->shouldDrawFogReturnAddress)
            return hooks->clientMode.callOriginal<bool, 17>();
    }
#endif
    
    return !Visuals::shouldRemoveFog();
}

static bool STDCALL_CONV shouldDrawViewModel(LINUX_ARGS(void* thisptr)) noexcept
{
    if (Visuals::isZoomOn() && localPlayer && localPlayer->fov() < 45 && localPlayer->fovStart() < 45)
        return false;
    return hooks->clientMode.callOriginal<bool, WIN32_LINUX(27, 28)>();
}

static void STDCALL_CONV lockCursor() noexcept
{
    if (gui->isOpen())
        return interfaces->surface->unlockCursor();
    return hooks->surface.callOriginal<void, 67>();
}

static void STDCALL_CONV setDrawColor(LINUX_ARGS(void* thisptr,) int r, int g, int b, int a) noexcept
{
    if (Visuals::shouldRemoveScopeOverlay() && (RETURN_ADDRESS() == memory->scopeDust || RETURN_ADDRESS() == memory->scopeArc))
        a = 0;
    hooks->surface.callOriginal<void, WIN32_LINUX(15, 14)>(r, g, b, a);
}

static void STDCALL_CONV overrideView(LINUX_ARGS(void* thisptr,) ViewSetup* setup) noexcept
{
    if (localPlayer && !localPlayer->isScoped())
        setup->fov += Visuals::fov();
    setup->farZ += Visuals::farZ() * 10;
    hooks->clientMode.callOriginal<void, WIN32_LINUX(18, 19)>(setup);
}

struct RenderableInfo {
    Entity* renderable;
    std::byte pad[18];
    uint16_t flags;
    uint16_t flags2;
};

static int STDCALL_CONV listLeavesInBox(LINUX_ARGS(void* thisptr, ) const Vector& mins, const Vector& maxs, unsigned short* list, int listMax) noexcept
{
    if (Misc::shouldDisableModelOcclusion() && RETURN_ADDRESS() == memory->insertIntoTree) {
        if (const auto info = *reinterpret_cast<RenderableInfo**>(FRAME_ADDRESS() + WIN32_LINUX(0x18, 0x10 + 0x948)); info && info->renderable) {
            if (const auto ent = VirtualMethod::call<Entity*, WIN32_LINUX(7, 8)>(info->renderable - sizeof(std::uintptr_t)); ent && ent->isPlayer()) {
                constexpr float maxCoord = 16384.0f;
                constexpr float minCoord = -maxCoord;
                constexpr Vector min{ minCoord, minCoord, minCoord };
                constexpr Vector max{ maxCoord, maxCoord, maxCoord };
                return hooks->bspQuery.callOriginal<int, 6>(std::cref(min), std::cref(max), list, listMax);
            }
        }
    }

    return hooks->bspQuery.callOriginal<int, 6>(std::cref(mins), std::cref(maxs), list, listMax);
}

static void STDCALL_CONV render2dEffectsPreHud(LINUX_ARGS(void* thisptr,) void* viewSetup) noexcept
{
    Visuals::applyScreenEffects();
    Visuals::hitEffect();
    hooks->viewRender.callOriginal<void, WIN32_LINUX(39, 40)>(viewSetup);
}

static const DemoPlaybackParameters* STDCALL_CONV getDemoPlaybackParameters(LINUX_ARGS(void* thisptr)) noexcept
{
    const auto params = hooks->engine.callOriginal<const DemoPlaybackParameters*, WIN32_LINUX(218, 219)>();

    if (params && Misc::shouldRevealSuspect() && RETURN_ADDRESS() != memory->demoFileEndReached) {
        static DemoPlaybackParameters customParams;
        customParams = *params;
        customParams.anonymousPlayerIdentity = false;
        return &customParams;
    }

    return params;
}

static bool STDCALL_CONV isPlayingDemo(LINUX_ARGS(void* thisptr)) noexcept
{
    if (Misc::shouldRevealMoney() && RETURN_ADDRESS() == memory->demoOrHLTV && *reinterpret_cast<std::uintptr_t*>(FRAME_ADDRESS() + WIN32_LINUX(8, 24)) == memory->money)
        return true;

    return hooks->engine.callOriginal<bool, 82>();
}

static void STDCALL_CONV updateColorCorrectionWeights(LINUX_ARGS(void* thisptr)) noexcept
{
    hooks->clientMode.callOriginal<void, WIN32_LINUX(58, 61)>();

    Visuals::performColorCorrection();
    if (Visuals::shouldRemoveScopeOverlay())
        *memory->vignette = 0.0f;
}

static float STDCALL_CONV getScreenAspectRatio(LINUX_ARGS(void* thisptr,) int width, int height) noexcept
{
    if (Misc::aspectRatio() != 0.0f)
        return Misc::aspectRatio();
    return hooks->engine.callOriginal<float, 101>(width, height);
}

static void STDCALL_CONV renderSmokeOverlay(LINUX_ARGS(void* thisptr,) bool update) noexcept
{
    if (Visuals::shouldRemoveSmoke() || Visuals::isSmokeWireframe())
        *reinterpret_cast<float*>(std::uintptr_t(memory->viewRender) + WIN32_LINUX(0x588, 0x648)) = 0.0f;
    else
        hooks->viewRender.callOriginal<void, WIN32_LINUX(41, 42)>(update);
}

static double STDCALL_CONV getArgAsNumber(LINUX_ARGS(void* thisptr,) void* params, int index) noexcept
{
    const auto result = hooks->panoramaMarshallHelper.callOriginal<double, 5>(params, index);
    inventory_changer::InventoryChanger::instance().getArgAsNumberHook(static_cast<int>(result), RETURN_ADDRESS());
    return result;
}

static const char* STDCALL_CONV getArgAsString(LINUX_ARGS(void* thisptr,) void* params, int index) noexcept
{
    const auto result = hooks->panoramaMarshallHelper.callOriginal<const char*, 7>(params, index);

    if (result)
        inventory_changer::InventoryChanger::instance().getArgAsStringHook(result, RETURN_ADDRESS(), params);

    return result;
}

static void STDCALL_CONV setResultInt(LINUX_ARGS(void* thisptr, ) void* params, int result) noexcept
{
    result = inventory_changer::InventoryChanger::instance().setResultIntHook(RETURN_ADDRESS(), params, result);
    hooks->panoramaMarshallHelper.callOriginal<void, WIN32_LINUX(14, 11)>(params, result);
}

static unsigned STDCALL_CONV getNumArgs(LINUX_ARGS(void* thisptr, ) void* params) noexcept
{
    const auto result = hooks->panoramaMarshallHelper.callOriginal<unsigned, 1>(params);
    inventory_changer::InventoryChanger::instance().getNumArgsHook(result, RETURN_ADDRESS(), params);
    return result;
}

static void STDCALL_CONV updateInventoryEquippedState(LINUX_ARGS(void* thisptr, ) CSPlayerInventory* inventory, std::uint64_t itemID, csgo::Team team, int slot, bool swap) noexcept
{
    inventory_changer::InventoryChanger::instance().onItemEquip(team, slot, itemID);
    return hooks->inventoryManager.callOriginal<void, WIN32_LINUX(29, 30)>(inventory, itemID, team, slot, swap);
}

static void STDCALL_CONV soUpdated(LINUX_ARGS(void* thisptr, ) SOID owner, SharedObject* object, int event) noexcept
{
    inventory_changer::InventoryChanger::instance().onSoUpdated(object);
    hooks->inventory.callOriginal<void, 1>(owner, object, event);
}

static bool STDCALL_CONV dispatchUserMessage(LINUX_ARGS(void* thisptr, ) csgo::UserMessageType type, int passthroughFlags, int size, const void* data) noexcept
{
    if (type == csgo::UserMessageType::Text)
        inventory_changer::InventoryChanger::instance().onUserTextMsg(data, size);
    else if (type == csgo::UserMessageType::VoteStart)
        Misc::onVoteStart(data, size);
    else if (type == csgo::UserMessageType::VotePass)
        Misc::onVotePass();
    else if (type == csgo::UserMessageType::VoteFailed)
        Misc::onVoteFailed();
    
    return hooks->client.callOriginal<bool, 38>(type, passthroughFlags, size, data);
}

static void FASTCALL_CONV packetEnd() noexcept
{
    const auto soundMessages = memory->soundMessages;
    if (soundMessages->numElements > 0) {
        for (int i = 0; i <= soundMessages->lastAlloc; ++i) {
            if (!soundMessages->isIndexUsed(i))
                continue;

            auto& soundInfo = soundMessages->memory[i].element;
            if (const char* soundName = interfaces->soundEmitter->getSoundName(soundInfo.soundIndex)) {
                Sound::modulateSound(soundName, soundInfo.entityIndex, soundInfo.volume);
                soundInfo.volume = std::clamp(soundInfo.volume, 0.0f, 1.0f);
            }
        }
    }

    hooks->clientState.callOriginal<void, 6>();
}

static void FASTCALL_CONV processPacket(void* thisptr, void* edx, void* packet, bool header) noexcept
{
    if (!CSGOUtils::getClientState()->netChannel)
        return hooks->clientState.callOriginal<void, void*, 39>(thisptr, packet, header);

    hooks->clientState.callOriginal<void, void*, 39>(thisptr, packet, header);

    for (auto it = CSGOUtils::getClientState()->events; it != nullptr; it = it->next) {
        if (!it->classID)
            continue;

        // set all delays to instant.
        it->fireDelay = 0.f;
    }

    interfaces->engine->fireEvents();
}

#ifdef _WIN32

static void FASTCALL_CONV standardBlendingRulesHook(void* thisPointer, void* edx, void* hdr, void* pos, void* q, float currentTime, int boneMask) noexcept
{
    const auto entity = reinterpret_cast<Entity*>(thisPointer);

    entity->getEffects() |= 8;

    hooks->standardBlendingRules.callOriginal<void, void*>(thisPointer, hdr, pos, q, currentTime, boneMask);

    entity->getEffects() &= ~8;
}

static bool FASTCALL_CONV shouldSkipAnimationFrameHook(void* thisPointer, void* edx) noexcept
{
    return false;
}

static void FASTCALL_CONV checkForSequenceChangeHook(void* thisPointer, void* edx, void* hdr, int currentSequence, bool forceNewSequence, bool interpolate) noexcept
{
    return hooks->checkForSequenceChange.callOriginal<void, void*>(thisPointer, hdr, currentSequence, forceNewSequence, true);
}

static void FASTCALL_CONV particleCollectionSimulateHook(ParticleCollection* thisptr) noexcept
{
    hooks->particleCollectionSimulate.callOriginal<void, ParticleCollection*>(thisptr);

    if (!interfaces->engine->isConnected())
        return;

    ParticleCollection* rootCollection = thisptr;
    while (rootCollection->parent)
        rootCollection = rootCollection->parent;

    const char* rootName = rootCollection->def.object->name.buffer;

    ColorToggle molotov = Visuals::getMolotovColor();
    ColorToggle smoke = Visuals::getSmokeColor();

    switch (fnv::hash(rootName))
    {
    case fnv::hash("molotov_groundfire"):
    case fnv::hash("molotov_groundfire_00MEDIUM"):
    case fnv::hash("molotov_groundfire_00HIGH"):
    case fnv::hash("molotov_groundfire_fallback"):
    case fnv::hash("molotov_groundfire_fallback2"):
    case fnv::hash("molotov_explosion"):
    case fnv::hash("explosion_molotov_air"):
    case fnv::hash("extinguish_fire"):
    case fnv::hash("weapon_molotov_held"):
    case fnv::hash("weapon_molotov_fp"):
    case fnv::hash("weapon_molotov_thrown"):
    case fnv::hash("incgrenade_thrown_trail"):
        switch (fnv::hash(thisptr->def.object->name.buffer))
        {
        case fnv::hash("explosion_molotov_air_smoke"):
        case fnv::hash("molotov_smoking_ground_child01"):
        case fnv::hash("molotov_smoking_ground_child02"):
        case fnv::hash("molotov_smoking_ground_child02_cheapo"):
        case fnv::hash("molotov_smoking_ground_child03"):
        case fnv::hash("molotov_smoking_ground_child03_cheapo"):
        case fnv::hash("molotov_smoke_screen"):
            break;
        default:
            if (molotov.enabled) {

                for (int i = 0; i < thisptr->activeParticles; i++) {
                    float* color = thisptr->particleAttributes.FloatAttributePtr(PARTICLE_ATTRIBUTE_TINT_RGB, i);
                    color[0] = molotov.asColor4().color[0];
                    color[4] = molotov.asColor4().color[1];
                    color[8] = molotov.asColor4().color[2];
                    float* alpha = thisptr->particleAttributes.FloatAttributePtr(PARTICLE_ATTRIBUTE_ALPHA, i);
                    *alpha = molotov.asColor4().color[3];
                }
            }
            break;
        }
        break;
    case fnv::hash("explosion_smokegrenade_fallback"):
    case fnv::hash("explosion_smokegrenade_CT"):
    case fnv::hash("explosion_smokegrenade_T"):
    case fnv::hash("explosion_smokegrenade"): {
        if (smoke.enabled) {

            for (int i = 0; i < thisptr->activeParticles; i++) {
                float* color = thisptr->particleAttributes.FloatAttributePtr(PARTICLE_ATTRIBUTE_TINT_RGB, i);
                color[0] = smoke.asColor4().color[0];
                color[4] = smoke.asColor4().color[1];
                color[8] = smoke.asColor4().color[2];
                float* alpha = thisptr->particleAttributes.FloatAttributePtr(PARTICLE_ATTRIBUTE_ALPHA, i);
                *alpha = smoke.asColor4().color[3];
            }
        }
    }
    }
}

static int FASTCALL_CONV sendDatagramHook(NetworkChannel* network, void* edx, void* datagram)
{
    if (!Backtrack::fakePing() || datagram || !interfaces->engine->isInGame())
        return hooks->sendDatagram.callOriginal<int, NetworkChannel*>(network, datagram);

    int instate = network->inReliableState;
    int insequencenr = network->inSequenceNr;

    float delta = (std::max)(0.f, Backtrack::pingAmount() / 1000.f);

    Backtrack::addLatencyToNetwork(network, delta);

    int result = hooks->sendDatagram.callOriginal<int, NetworkChannel*>(network, datagram);

    network->inReliableState = instate;
    network->inSequenceNr = insequencenr;

    return result;
}

static void FASTCALL_CONV physicsSimulateHook(void* thisptr, void* edx) noexcept
{
    const auto entity = reinterpret_cast<Entity*>(thisptr);
    if (!localPlayer || !localPlayer->isAlive() || entity != localPlayer.get())
        return hooks->physicsSimulate.callOriginal<void, void*>(thisptr);

    const int simulationTick = *reinterpret_cast<int*>(reinterpret_cast<uintptr_t>(thisptr) + 0x2AC);
    if (simulationTick == memory->globalVars->tickCount)
        return;

    CommandContext* commandContext = localPlayer->getCommandContext();
    if (!commandContext || !commandContext->needsProcessing)
        return;

    hooks->physicsSimulate.callOriginal<void, void*>(thisptr);
    EnginePrediction::store();
}

/*
static bool STDCALL_CONV isDepthOfFieldEnabledHook() noexcept
{
    Visuals::motionBlur(nullptr);
    return false;
}
*/

static int STDCALL_CONV getUnverifiedFileHashes(void* thisptr, int maxFiles) noexcept
{
    if (Misc::shouldEnableSvPureBypass())
        return 0;

    return hooks->fileSystem.callOriginal<int, 101>(thisptr, maxFiles);
}

static int FASTCALL_CONV canLoadThirdPartyFiles(void* thisptr, void* edx) noexcept
{
    if (Misc::shouldEnableSvPureBypass())
        return 1;

    return hooks->fileSystem.callOriginal<int, void*, 128>(thisptr);
}

static bool FASTCALL_CONV isConnected() noexcept
{
    if (Misc::shouldInvUnlock() && RETURN_ADDRESS() == memory->isLoadoutAllowed)
        return false;

    return hooks->engine.callOriginal<bool, 27>();
}

static Result FASTCALL_CONV retrieveMessage(void* ecx, void* edx, uint32_t* messageType, void* pubDest, uint32_t cubDest, uint32_t* messageSize)
{
    const auto status = hooks->gameCoordinator.callOriginal<Result, void*, 2>(ecx, messageType, pubDest, cubDest, messageSize);

    if (ProfileChanger::isEnabled() && status == Result::OK)
    {
        void* thisPtr = nullptr;
        __asm mov thisPtr, ebx;
        auto oldEBP = *reinterpret_cast<void**>(FRAME_ADDRESS() - 4);

        uint32_t msgType = *messageType & 0x7FFFFFFF;
        write.receiveMessage(thisPtr, oldEBP, msgType, pubDest, cubDest, messageSize);
    }

    return status;
}

static void* STDCALL_CONV allocKeyValuesMemory(LINUX_ARGS(void* thisptr, ) int size) noexcept
{
    if (const auto returnAddress = RETURN_ADDRESS(); returnAddress == memory->keyValuesAllocEngine || returnAddress == memory->keyValuesAllocClient)
        return nullptr;

    return hooks->keyValuesSystem.callOriginal<void*, 2>(size);
}

static char STDCALL_CONV HookedHost_IsSecureServerAllowed() {
    if (Misc::shouldBypassInsecure())
        return true;

    return hooks->isSecureServerAllowed.callOriginal<char>();
}

Hooks::Hooks(HMODULE moduleHandle) noexcept : moduleHandle{ moduleHandle }
{
#ifndef __MINGW32__
    _MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
    _MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_ON);
#endif

    // interfaces and memory shouldn't be initialized in wndProc because they show MessageBox on error which would cause deadlock
    interfaces.emplace(Interfaces{});
    memory.emplace(Memory{});

    window = FindWindowW(L"Valve001", nullptr);
    if (!window)
        window = WindowUtils::GetMainWindow();
    originalWndProc = WNDPROC(SetWindowLongPtrW(window, GWLP_WNDPROC, LONG_PTR(&wndProc)));
}

#endif

void Hooks::install() noexcept
{
#ifdef _WIN32
    originalPresent = **reinterpret_cast<decltype(originalPresent)**>(memory->present);
    **reinterpret_cast<decltype(present)***>(memory->present) = present;
    originalReset = **reinterpret_cast<decltype(originalReset)**>(memory->reset);
    **reinterpret_cast<decltype(reset)***>(memory->reset) = reset;

    if constexpr (std::is_same_v<HookType, MinHook>)
        MH_Initialize();
#else
    ImGui_ImplOpenGL3_Init();

    swapWindow = *reinterpret_cast<decltype(swapWindow)*>(memory->swapWindow);
    *reinterpret_cast<decltype(::swapWindow)**>(memory->swapWindow) = ::swapWindow;

#endif

#ifdef _WIN32
    particleCollectionSimulate.detour(memory->particleCollection, particleCollectionSimulateHook);
    sendDatagram.detour(memory->sendDatagram, sendDatagramHook);
    standardBlendingRules.detour(memory->standardBlendingRules, standardBlendingRulesHook);
    shouldSkipAnimationFrame.detour(memory->shouldSkipAnimationFrame, shouldSkipAnimationFrameHook);
    checkForSequenceChange.detour(memory->checkForSequenceChange, checkForSequenceChangeHook);
    isSecureServerAllowed.detour(memory->isSecureServerAllowed, HookedHost_IsSecureServerAllowed);
#endif
    
    bspQuery.init(interfaces->engine->getBSPTreeQuery());
    bspQuery.hookAt(6, &listLeavesInBox);

    client.init(interfaces->client);
#ifdef _WIN32
    client.hookAt(22, &createMoveProxy);
#endif
    client.hookAt(37, &frameStageNotify);
    client.hookAt(38, &dispatchUserMessage);

    clientMode.init(memory->clientMode);
    clientMode.hookAt(WIN32_LINUX(17, 18), &shouldDrawFog);
    clientMode.hookAt(WIN32_LINUX(18, 19), &overrideView);
#ifndef _WIN32
    clientMode.hookAt(WIN32_LINUX(24, 25), &createMove);
#endif
    clientMode.hookAt(WIN32_LINUX(27, 28), &shouldDrawViewModel);
    clientMode.hookAt(WIN32_LINUX(35, 36), &getViewModelFov);
    clientMode.hookAt(WIN32_LINUX(44, 45), &doPostScreenEffects);
    clientMode.hookAt(WIN32_LINUX(58, 61), &updateColorCorrectionWeights);

    clientState.init(&memory->splitScreen->splitScreenPlayers[0]->client);
    clientState.hookAt(6, packetEnd);
    clientState.hookAt(39, processPacket);

    engine.init(interfaces->engine);
#ifdef _WIN32
    engine.hookAt(27, &isConnected);
#endif
    engine.hookAt(82, &isPlayingDemo);
    engine.hookAt(101, &getScreenAspectRatio);
#ifdef _WIN32
    fileSystem.init(interfaces->fileSystem);
    fileSystem.hookAt(101, &getUnverifiedFileHashes);
    fileSystem.hookAt(128, &canLoadThirdPartyFiles);

    gameCoordinator.init(memory->steamGameCoordinator);
    gameCoordinator.hookAt(2, retrieveMessage);

    keyValuesSystem.init(memory->keyValuesSystem);
    keyValuesSystem.hookAt(2, &allocKeyValuesMemory);
#endif
    engine.hookAt(WIN32_LINUX(218, 219), &getDemoPlaybackParameters);

    inventory.init(memory->inventoryManager->getLocalInventory());
    inventory.hookAt(1, &soUpdated);

    inventoryManager.init(memory->inventoryManager);
    inventoryManager.hookAt(WIN32_LINUX(29, 30), &updateInventoryEquippedState);

    modelRender.init(interfaces->modelRender);
    modelRender.hookAt(21, &drawModelExecute);

    panoramaMarshallHelper.init(memory->panoramaMarshallHelper);
    panoramaMarshallHelper.hookAt(1, &getNumArgs);
    panoramaMarshallHelper.hookAt(5, &getArgAsNumber);
    panoramaMarshallHelper.hookAt(7, &getArgAsString);
    panoramaMarshallHelper.hookAt(WIN32_LINUX(14, 11), &setResultInt);

    sound.init(interfaces->sound);
    sound.hookAt(WIN32_LINUX(5, 6), &emitSound);

    surface.init(interfaces->surface);
    surface.hookAt(WIN32_LINUX(15, 14), &setDrawColor);
    
    svCheats.init(interfaces->cvar->findVar("sv_cheats"));
    svCheats.hookAt(WIN32_LINUX(13, 16), &svCheatsGetBool);

    viewRender.init(memory->viewRender);
    viewRender.hookAt(WIN32_LINUX(39, 40), &render2dEffectsPreHud);
    viewRender.hookAt(WIN32_LINUX(41, 42), &renderSmokeOverlay);

#ifdef _WIN32
    surface.hookAt(67, &lockCursor);

    if constexpr (std::is_same_v<HookType, MinHook>)
        MH_EnableHook(MH_ALL_HOOKS);
#endif
}

#ifdef _WIN32

extern "C" BOOL WINAPI _CRT_INIT(HMODULE moduleHandle, DWORD reason, LPVOID reserved);

static DWORD WINAPI unload(HMODULE moduleHandle) noexcept
{
    Sleep(100);

    interfaces->inputSystem->enableInput(true);
    EventListener::remove();

    ImGui_ImplDX9_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    // Restore PE Header/PEB
    antiDetection.reset();

    _CRT_INIT(moduleHandle, DLL_PROCESS_DETACH, nullptr);

    FreeLibraryAndExitThread(moduleHandle, 0);
}

#endif

void Hooks::uninstall() noexcept
{
    Misc::updateEventListeners(true);
    Visuals::updateEventListeners(true);

#ifdef _WIN32
    if constexpr (std::is_same_v<HookType, MinHook>) {
        MH_DisableHook(MH_ALL_HOOKS);
        MH_Uninitialize();
    }
#endif

    bspQuery.restore();
    client.restore();
    clientMode.restore();
    engine.restore();
    inventory.restore();
    inventoryManager.restore();
    modelRender.restore();
    panoramaMarshallHelper.restore();
    sound.restore();
    surface.restore();
    svCheats.restore();
    viewRender.restore();

    Netvars::restore();

    Glow::clearCustomObjects();
    inventory_changer::InventoryChanger::instance().reset();

#ifdef _WIN32
    fileSystem.restore();
    gameCoordinator.restore();
    keyValuesSystem.restore();

    SetWindowLongPtrW(window, GWLP_WNDPROC, LONG_PTR(originalWndProc));
    **reinterpret_cast<void***>(memory->present) = originalPresent;
    **reinterpret_cast<void***>(memory->reset) = originalReset;

    if (HANDLE thread = CreateThread(nullptr, 0, LPTHREAD_START_ROUTINE(unload), moduleHandle, 0, nullptr))
        CloseHandle(thread);
#else
    *reinterpret_cast<decltype(pollEvent)*>(memory->pollEvent) = pollEvent;
    *reinterpret_cast<decltype(swapWindow)*>(memory->swapWindow) = swapWindow;
#endif
}

void Hooks::callOriginalDrawModelExecute(void* ctx, void* state, const ModelRenderInfo& info, matrix3x4* customBoneToWorld) noexcept
{
    modelRender.callOriginal<void, 21>(ctx, state, std::cref(info), customBoneToWorld);
}

#ifndef _WIN32

static int pollEvent(SDL_Event* event) noexcept
{
    [[maybe_unused]] static const auto once = []() noexcept {
        Netvars::init();
        EventListener::init();

        ImGui::CreateContext();
        config.emplace(Config{});

        gui.emplace(GUI{});

        hooks->install();

        return true;
    }();

    const auto result = hooks->pollEvent(event);

    if (result && ImGui_ImplSDL2_ProcessEvent(event) && gui->isOpen())
        event->type = 0;

    return result;
}

Hooks::Hooks() noexcept
{
    interfaces.emplace(Interfaces{});
    memory.emplace(Memory{});

    pollEvent = *reinterpret_cast<decltype(pollEvent)*>(memory->pollEvent);
    *reinterpret_cast<decltype(::pollEvent)**>(memory->pollEvent) = ::pollEvent;
}

#endif
