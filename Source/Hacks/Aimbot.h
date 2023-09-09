#pragma once

struct UserCmd;
struct Vector;

namespace Aimbot
{
    Vector calculateRelativeAngle(const Vector& source, const Vector& destination, const Vector& viewAngles) noexcept;
    void fixMouseDelta(UserCmd* cmd) noexcept;
    void run(UserCmd*) noexcept;

    void updateInput() noexcept;
}
