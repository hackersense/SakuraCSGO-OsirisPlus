#pragma once

#include "Pad.h"
#include "Vector.h"
#include "UserCmd.h"

class Input {
public:
    PAD(WIN32_LINUX(12, 16))
    bool isTrackIRAvailable;
    bool isMouseInitialized;
    bool isMouseActive;
    PAD(WIN32_LINUX(154, 162))
    bool isCameraInThirdPerson;
    bool cameraMovingWithMouse;
    Vector cameraOffset;
    PAD(56)
    UserCmd* commands;
    VerifiedUserCmd* verifiedCommands;

    UserCmd* getUserCmd(int sequenceNumber) noexcept
    {
        return &commands[sequenceNumber % 150];
    }

    VerifiedUserCmd* getVerifiedUserCmd(int sequenceNumber) noexcept
    {
        return &verifiedCommands[sequenceNumber % 150];
    }
};
