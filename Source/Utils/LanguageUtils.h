﻿#pragma once

#include <string>

enum class LanguageID
{
	GUI_TAB_AIMBOT,
	GUI_TAB_ANTIAIM,
	GUI_TAB_TRIGGERBOT,
	GUI_TAB_BACKTRACK,
	GUI_TAB_GLOW,
	GUI_TAB_CHAMS,
	GUI_TAB_ESP,
	GUI_TAB_VISUALS,
	GUI_TAB_PROFILECHANGER,
	GUI_TAB_INVENTORYCHANGER,
	GUI_TAB_SOUND,
	GUI_TAB_STYLE,
	GUI_TAB_MISC,
	GUI_TAB_CONFIG,
};

enum class LanguageType
{
	English,
	Chinese,
};

namespace Language
{
	void init() noexcept;
	const char* getText(LanguageID textID, LanguageType language) noexcept;
	const char* getText(LanguageID textID) noexcept;
}