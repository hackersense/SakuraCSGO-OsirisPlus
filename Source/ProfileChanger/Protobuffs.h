#pragma once
#include <windows.h>
#include <string>

class Protobuffs
{
public:
	static void writePacket(std::string packet, void* thisPtr, void* oldEBP, void* pubDest, uint32_t cubDest, uint32_t* pcubMsgSize);
	bool sendClientGcRankUpdateMatchmaking();
	bool sendClientGcRankUpdateWingman();
	bool sendClientGcRankUpdateZone();
	void receiveMessage(void* thisPtr, void* oldEBP, uint32_t messageType, void* pubDest, uint32_t cubDest, uint32_t* pcubMsgSize);
	bool sendClientHello();
	bool sendClientHelloFix();
};

inline Protobuffs write;