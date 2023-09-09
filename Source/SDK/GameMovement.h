#pragma once

#include "Inconstructible.h"
#include "VirtualMethod.h"

class Entity;
class MoveData;

class GameMovement {
public:
    INCONSTRUCTIBLE(GameMovement)

    VIRTUAL_METHOD_V(void, processMovement, 1, (Entity* localPlayer, MoveData* moveData), (this, localPlayer, moveData))
    VIRTUAL_METHOD_V(void, reset, 2, (), (this))
    VIRTUAL_METHOD_V(void, startTrackPredictionErrors, 3, (Entity* localPlayer), (this, localPlayer))
    VIRTUAL_METHOD_V(void, finishTrackPredictionErrors, 4, (Entity* localPlayer), (this, localPlayer))
    VIRTUAL_METHOD(Vector&, getPlayerViewOffset, 8, (bool ducked), (this, ducked))
};
