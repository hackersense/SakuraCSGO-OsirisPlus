#pragma once

#include "Vector.h"
#include "CheckSumCRC.h"

struct UserCmd {
    enum {
        IN_ATTACK = 1 << 0,
        IN_JUMP = 1 << 1,
        IN_DUCK = 1 << 2,
        IN_FORWARD = 1 << 3,
        IN_BACK = 1 << 4,
        IN_USE = 1 << 5,
        IN_CANCEL = 1 << 6,
        IN_LEFT = 1 << 7,
        IN_RIGHT = 1 << 8,
        IN_MOVELEFT = 1 << 9,
        IN_MOVERIGHT = 1 << 10,
        IN_ATTACK2 = 1 << 11,
        IN_RUN = 1 << 12,
        IN_RELOAD = 1 << 13,
        IN_ALT1 = 1 << 14,
        IN_ALT2 = 1 << 15,
        IN_SCORE = 1 << 16,
        IN_SPEED = 1 << 17,
        IN_WALK = 1 << 18,
        IN_ZOOM = 1 << 19,
        IN_WEAPON1 = 1 << 20,
        IN_WEAPON2 = 1 << 21,
        IN_BULLRUSH = 1 << 22,
        IN_GRENADE1 = 1 << 23,
        IN_GRENADE2 = 1 << 24,
        IN_LOOKSPIN = 1 << 25
    };

    CRC32 getChecksum() const noexcept
    {
        CRC32 crc;
        CRC32_Init(&crc);

        CRC32_ProcessBuffer(&crc, &commandNumber, sizeof(commandNumber));
        CRC32_ProcessBuffer(&crc, &tickCount, sizeof(tickCount));
        CRC32_ProcessBuffer(&crc, &viewangles, sizeof(viewangles));
        CRC32_ProcessBuffer(&crc, &aimdirection, sizeof(aimdirection));
        CRC32_ProcessBuffer(&crc, &forwardmove, sizeof(forwardmove));
        CRC32_ProcessBuffer(&crc, &sidemove, sizeof(sidemove));
        CRC32_ProcessBuffer(&crc, &upmove, sizeof(upmove));
        CRC32_ProcessBuffer(&crc, &buttons, sizeof(buttons));
        CRC32_ProcessBuffer(&crc, &impulse, sizeof(impulse));
        CRC32_ProcessBuffer(&crc, &weaponselect, sizeof(weaponselect));
        CRC32_ProcessBuffer(&crc, &weaponsubtype, sizeof(weaponsubtype));
        CRC32_ProcessBuffer(&crc, &randomSeed, sizeof(randomSeed));
        CRC32_ProcessBuffer(&crc, &mousedx, sizeof(mousedx));
        CRC32_ProcessBuffer(&crc, &mousedy, sizeof(mousedy));

        CRC32_Final(&crc);
        return crc;
    }

    void* vmt;
    int commandNumber;
    int tickCount;
    Vector viewangles;
    Vector aimdirection;
    float forwardmove;
    float sidemove;
    float upmove;
    int buttons;
    char impulse;
    int weaponselect;
    int weaponsubtype;
    int randomSeed;
    short mousedx;
    short mousedy;
    bool hasbeenpredicted;
    Vector viewanglesBackup;
    int buttonsBackup;
    std::byte pad[8];
};

struct VerifiedUserCmd
{
public:
    UserCmd cmd;
    CRC32 crc;
};
