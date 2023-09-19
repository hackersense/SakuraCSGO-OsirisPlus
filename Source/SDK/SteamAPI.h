#pragma once

#include <iostream>

enum class Result
{
	OK = 0,
	NoMessage = 1,           // There is no message in the queue
	BufferTooSmall = 2,      // The buffer is too small for the requested message
	NotLoggedOn = 3,         // The client is not logged onto Steam
	InvalidMessage = 4,      // Something was wrong with the message being sent with SendMessage
};

class GameCoordinator
{
public:
	virtual Result sendMessage(int unMsgType, const void* pubData, int cubData) = 0;
	virtual bool isMessageAvailable(int* pcubMsgSize) = 0;
	virtual Result retrieveMessage(int* punMsgType, void* pubDest, int cubDest, int* pcubMsgSize) = 0;
};

class SteamID
{
public:
	SteamID()
	{
		steamid.comp.unAccountID = 0;
		steamid.comp.accountType = 0;
		steamid.comp.universe = 0;
		steamid.comp.unAccountInstance = 0;
	}

	std::uint32_t getAccountID() const
	{
		return steamid.comp.unAccountID;
	}
private:
	union SteamIDInfo
	{
		struct SteamIDComponent
		{
			std::uint32_t			unAccountID : 32;			// unique account identifier
			std::uint32_t			unAccountInstance : 20;		// dynamic instance ID (used for multiseat type accounts only)
			std::uint32_t			accountType : 4;			// type of account - can't show as EAccountType, due to signed / unsigned difference
			int						universe : 8;				// universe this account belongs to
		} comp;

		std::uint64_t unAll64Bits;
	} steamid;
};

class User
{
public:
	virtual std::uint32_t getSteamUser() = 0;
	virtual bool loggedOn() = 0;
	virtual SteamID getSteamID() = 0;
};

class SteamClient
{
public:
	User* getUser(void* user, void* pipe, const char* pchVersion)
	{
		return (*reinterpret_cast<User*(THISCALL_CONV***)(void*, void*, void*, const char*)>(this))[5](this, user, pipe, pchVersion);
	}

	GameCoordinator* getGenericInterface(void* user, void* pipe, const char* pchVersion)
	{
		return (*reinterpret_cast<GameCoordinator*(THISCALL_CONV***)(void*, void*, void*, const char*)>(this))[12](this, user, pipe, pchVersion);
	}
};