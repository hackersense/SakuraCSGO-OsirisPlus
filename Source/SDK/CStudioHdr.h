#pragma once

#include "Pad.h"
#include "ModelInfo.h"

struct CStudioHdr
{
	StudioHdr* hdr;
	void* virtualModel;
	void* softBody;
	UtlVector<const StudioHdr*> hdrCache;
	int	numFrameUnlockCounter;
	int* frameUnlockCounter;
	PAD(8)
	UtlVector<int> boneFlags;
	UtlVector<int> boneParent;
	void* activityToSequence;
};