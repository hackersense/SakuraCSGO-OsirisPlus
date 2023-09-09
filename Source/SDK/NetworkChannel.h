#pragma once

#include "Inconstructible.h"
#include "VirtualMethod.h"

class NetworkChannel {
public:
    INCONSTRUCTIBLE(NetworkChannel)

	VIRTUAL_METHOD(const char*, getName, 0, (), (this))
	VIRTUAL_METHOD(const char*, getAddress, 1, (), (this))
	VIRTUAL_METHOD(float, getLatency, 9, (int flow), (this, flow))
	VIRTUAL_METHOD(void*, setTimeout, 31, (float newTimeoutSeconds, bool forceSet), (this, newTimeoutSeconds, forceSet))
	VIRTUAL_METHOD(unsigned int, requestFile, 62, (const char* filename, bool isReplayDemo), (this, filename, isReplayDemo))

    std::byte pad[24];
	int outSequenceNr;
	int inSequenceNr;
	int outSequenceNrAck;
	int outReliableState;
	int inReliableState;
	int chokedPackets;
};
