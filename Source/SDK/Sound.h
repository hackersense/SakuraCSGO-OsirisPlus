#pragma once

#include "Pad.h"
#include "Vector.h"

struct ActiveChannels {
    int count;
    short list[128];
};

struct SoundParams
{
	void* filter;
	int entityIndex = -1;
	int channel;
	const char* soundEntry;
	int soundEntryHash;
	const char* sample;
	float volume = 1.0f;
	int seed;
	int soundLevel;
	int flags;
	int pitch = 100;
	Vector* origin;
	Vector* direction;
	UtlVector<Vector>* origins;
	bool updatePosition;
	float soundTime = 0.0f;
	int speakerEntity = -1;
	void* soundParams;
};

struct Channel {
    PAD(WIN32_LINUX(244, 260))
    int soundSource;
    PAD(56)
    Vector origin;
    Vector direction;
    PAD(80)
};

struct ActiveSoundInfo
{
	int guid;
	void* fileNameHandle;
	int soundSource;
	int channel;
	int speakerEntity;
	float volume;
	float lastSpatializedVolume;
	float radius;
	int pitch;
	Vector* origin;
	Vector* direction;
	bool updatePositions;
	bool isSentence;
	bool dryMix;
	bool speaker;
	bool fromServer;
};

class EngineSound
{
public:
	INCONSTRUCTIBLE(EngineSound)

	VIRTUAL_METHOD(int, emitSound, 5, (SoundParams data), (this, data))
	VIRTUAL_METHOD(void, getActiveSounds, 19, (UtlVector<ActiveSoundInfo>& soundList), (this, std::ref(soundList)))
};