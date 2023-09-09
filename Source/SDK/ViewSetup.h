#pragma once

#include "Vector.h"
#include "Pad.h"

struct ViewSetup {
    PAD(172);
    void* csm;
    float fov;
    PAD(32);
    float farZ;
};