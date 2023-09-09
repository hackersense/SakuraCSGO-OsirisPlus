#pragma once

#include <cstddef>

class Entity;

struct AnimState {
    std::byte pad[3];
    bool firstRunSinceInit;
    std::byte pad1[87];
    Entity* lastBoneSetupWeapon;
    Entity* baseEntity;
    Entity* activeWeapon;
    Entity* lastActiveWeapon;
    float lastClientSideAnimationUpdateTime;
    int lastClientSideAnimationUpdateFramecount;
    float updateTimeDelta;
    float eyeYaw;
    float eyePitch;
    float goalFeetYaw;
    float currentFeetYaw;
    float currentTorsoYaw;
    float idealVelocityLean; //changes when moving/jumping/hitting ground
    float leanAmount;
    float timeToAlignLowerBody;
    float feetCycle;
    float feetYawRate;
    float feetYawRateSmoothed;
    float duckAmount;
    std::byte pad2[80];
    float footSpeed;
    float footSpeed2;
    std::byte pad3[22];
    float stopToFullRunningFraction;
    std::byte pad4[532];
    float velocitySubtractY;
};
